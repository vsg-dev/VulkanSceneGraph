/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/Version.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/raytracing/RayTracingPipeline.h>
#include <vsg/state/ComputePipeline.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/utils/ShaderCompiler.h>

#if VSG_SUPPORTS_ShaderCompiler
#    include <glslang/Public/ResourceLimits.h>
#    include <glslang/Public/ShaderLang.h>
#    include <glslang/SPIRV/GlslangToSpv.h>
#endif

#if VSG_SUPPORTS_ShaderOptimizer
#    include <spirv-tools/optimizer.hpp>
#endif

#include <algorithm>
#include <cassert>
#include <iomanip>

#ifndef VK_API_VERSION_MAJOR
#    define VK_API_VERSION_MAJOR(version) (((uint32_t)(version) >> 22) & 0x7FU)
#    define VK_API_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3FFU)
#endif

using namespace vsg;

namespace
{
#if VSG_SUPPORTS_ShaderCompiler
    static std::atomic_uint s_initialized = 0;

    static void s_initializeProcess()
    {
        if (s_initialized.fetch_add(1) == 0)
        {
            glslang::InitializeProcess();
        }
    }

    static void s_finalizeProcess()
    {
        if (s_initialized.fetch_sub(1) == 1)
        {
            glslang::FinalizeProcess();
        }
    }

#endif

#if VSG_SUPPORTS_ShaderOptimizer
    constexpr static spv_target_env selectTargetEnv(const ShaderCompileSettings& settings)
    {
        switch (settings.vulkanVersion)
        {
        case VK_MAKE_VERSION(0, 1, 0):
            return SPV_ENV_VULKAN_1_0;
        case VK_MAKE_VERSION(0, 1, 1):
            switch (settings.target)
            {
            case ShaderCompileSettings::SPIRV_1_0:
            case ShaderCompileSettings::SPIRV_1_1:
            case ShaderCompileSettings::SPIRV_1_2:
            case ShaderCompileSettings::SPIRV_1_3:
                return SPV_ENV_VULKAN_1_1;
            case ShaderCompileSettings::SPIRV_1_4:
                return SPV_ENV_VULKAN_1_1_SPIRV_1_4;
            default:
                return SPV_ENV_VULKAN_1_1;
            }
        case VK_MAKE_VERSION(0, 1, 2):
            return SPV_ENV_VULKAN_1_2;
        case VK_MAKE_VERSION(0, 1, 3):
            return SPV_ENV_VULKAN_1_3;
        //case VK_MAKE_VERSION(0, 1, 4):
        //    return SPV_ENV_VULKAN_1_4;
        default:
            return SPV_ENV_UNIVERSAL_1_0;
        }
    }
#endif
} // namespace

std::string debugFormatShaderSource(const std::string& source)
{
    std::istringstream iss(source);
    std::ostringstream oss;

    uint32_t linecount = 1;

    for (std::string line; std::getline(iss, line);)
    {
        oss << std::setw(4) << std::setfill(' ') << linecount << ": " << line << "\n";
        linecount++;
    }
    return oss.str();
}

ShaderCompiler::ShaderCompiler() :
    Inherit(),
    defaults(ShaderCompileSettings::create())
{
}

ShaderCompiler::~ShaderCompiler()
{
#if VSG_SUPPORTS_ShaderCompiler
    s_finalizeProcess();
#endif
}

bool ShaderCompiler::supported() const
{
    return VSG_SUPPORTS_ShaderCompiler == 1;
}

#if VSG_SUPPORTS_ShaderCompiler
bool ShaderCompiler::compile(ShaderStages& shaders, const std::vector<std::string>& defines, ref_ptr<const Options> options)
{
    // need to balance the inits.
    if (!_initialized)
    {
        s_initializeProcess();
        _initialized = true;
    }

    auto getFriendlyNameForShader = [](const ref_ptr<ShaderStage>& vsg_shader) {
        switch (vsg_shader->stage)
        {
        case (VK_SHADER_STAGE_VERTEX_BIT): return "Vertex Shader";
        case (VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT): return "Tessellation Control Shader";
        case (VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT): return "Tessellation Evaluation Shader";
        case (VK_SHADER_STAGE_GEOMETRY_BIT): return "Geometry Shader";
        case (VK_SHADER_STAGE_FRAGMENT_BIT): return "Fragment Shader";
        case (VK_SHADER_STAGE_COMPUTE_BIT): return "Compute Shader";
        case (VK_SHADER_STAGE_RAYGEN_BIT_KHR): return "RayGen Shader";
        case (VK_SHADER_STAGE_ANY_HIT_BIT_KHR): return "Any Hit Shader";
        case (VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR): return "Closest Hit Shader";
        case (VK_SHADER_STAGE_MISS_BIT_KHR): return "Miss Shader";
        case (VK_SHADER_STAGE_INTERSECTION_BIT_KHR): return "Intersection Shader";
        case (VK_SHADER_STAGE_CALLABLE_BIT_KHR): return "Callable Shader";
        case (VK_SHADER_STAGE_TASK_BIT_EXT): return "Task Shader";
        case (VK_SHADER_STAGE_MESH_BIT_EXT): return "Mesh Shader";
        default: return "Unknown Shader Type";
        }
        return "";
    };

    using StageShaderMap = std::map<EShLanguage, ref_ptr<ShaderStage>>;
    using TShaders = std::list<std::unique_ptr<glslang::TShader>>;
    TShaders tshaders;

    auto builtInResources = GetDefaultResources();

#    if VSG_SUPPORTS_ShaderOptimizer
    auto optimizer = std::make_unique<spvtools::Optimizer>(selectTargetEnv(*defaults));
    optimizer->SetMessageConsumer([](spv_message_level_t level, const char* source, const spv_position_t& position, const char* message) {
        Logger::Level vsgLevel;
        switch (level)
        {
        case SPV_MSG_FATAL:
        case SPV_MSG_INTERNAL_ERROR:
            vsgLevel = Logger::LOGGER_FATAL;
            break;
        case SPV_MSG_ERROR:
            vsgLevel = Logger::LOGGER_ERROR;
            break;
        case SPV_MSG_WARNING:
            vsgLevel = Logger::LOGGER_WARN;
            break;
        case SPV_MSG_INFO:
            vsgLevel = Logger::LOGGER_INFO;
            break;
        case SPV_MSG_DEBUG:
            vsgLevel = Logger::LOGGER_DEBUG;
            break;
        default:
            vsgLevel = Logger::LOGGER_INFO;
            break;
        }
        vsg::log(vsgLevel, "spvtools::Optimizer: ", source ? source : "", ", ", position.line, ", ", position.column, ", ", position.index, ", ", message ? message : "No message");
    });
#    endif

    StageShaderMap stageShaderMap;
    std::unique_ptr<glslang::TProgram> program(new glslang::TProgram);

    for (auto& vsg_shader : shaders)
    {
        EShLanguage envStage = EShLangCount;

        glslang::EShTargetLanguageVersion minTargetLanguageVersion = glslang::EShTargetSpv_1_0;

        switch (vsg_shader->stage)
        {
        case (VK_SHADER_STAGE_VERTEX_BIT): envStage = EShLangVertex; break;
        case (VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT): envStage = EShLangTessControl; break;
        case (VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT): envStage = EShLangTessEvaluation; break;
        case (VK_SHADER_STAGE_GEOMETRY_BIT): envStage = EShLangGeometry; break;
        case (VK_SHADER_STAGE_FRAGMENT_BIT): envStage = EShLangFragment; break;
        case (VK_SHADER_STAGE_COMPUTE_BIT): envStage = EShLangCompute; break;
        case (VK_SHADER_STAGE_RAYGEN_BIT_KHR):
            envStage = EShLangRayGen;
            minTargetLanguageVersion = glslang::EShTargetSpv_1_4;
            break;
        case (VK_SHADER_STAGE_ANY_HIT_BIT_KHR):
            envStage = EShLangAnyHit;
            minTargetLanguageVersion = glslang::EShTargetSpv_1_4;
            break;
        case (VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR):
            envStage = EShLangClosestHit;
            minTargetLanguageVersion = glslang::EShTargetSpv_1_4;
            break;
        case (VK_SHADER_STAGE_MISS_BIT_KHR):
            envStage = EShLangMiss;
            minTargetLanguageVersion = glslang::EShTargetSpv_1_4;
            break;
        case (VK_SHADER_STAGE_INTERSECTION_BIT_KHR):
            envStage = EShLangIntersect;
            minTargetLanguageVersion = glslang::EShTargetSpv_1_4;
            break;
        case (VK_SHADER_STAGE_CALLABLE_BIT_KHR):
            envStage = EShLangCallable;
            minTargetLanguageVersion = glslang::EShTargetSpv_1_4;
            break;
        case (VK_SHADER_STAGE_TASK_BIT_EXT):
            envStage = EShLangTask;
            minTargetLanguageVersion = glslang::EShTargetSpv_1_4;
            break;
        case (VK_SHADER_STAGE_MESH_BIT_EXT):
            envStage = EShLangMesh;
            minTargetLanguageVersion = glslang::EShTargetSpv_1_4;
            break;

        default: break;
        }

        if (envStage == EShLangCount)
        {
            warn("ShaderCompiler::compile() unsupported stage : ", vsg_shader->stage);
            return false;
        }

        glslang::TShader* shader(new glslang::TShader(envStage));
        tshaders.emplace_back(shader);

        // select the ShaderCompileSettings to use
        auto settings = vsg_shader->module->hints ? vsg_shader->module->hints : defaults;

        // select the most appropriate Spirv version
        glslang::EShTargetLanguageVersion targetLanguageVersion = std::max(static_cast<glslang::EShTargetLanguageVersion>(settings->target), minTargetLanguageVersion);

        // convert Vulkan version to glsLang equivalent
        glslang::EShTargetClientVersion targetClientVersion = static_cast<glslang::EShTargetClientVersion>((VK_API_VERSION_MAJOR(settings->vulkanVersion) << 22) | (VK_API_VERSION_MINOR(settings->vulkanVersion) << 12));

        glslang::EShSource language = glslang::EShSourceNone;
        switch (settings->language)
        {
        case (ShaderCompileSettings::GLSL): language = glslang::EShSourceGlsl; break;
        case (ShaderCompileSettings::HLSL): language = glslang::EShSourceHlsl; break;
        }

        shader->setEnvInput(language, envStage, glslang::EShClientVulkan, settings->clientInputVersion);
        shader->setEnvClient(glslang::EShClientVulkan, targetClientVersion);
        shader->setEnvTarget(glslang::EShTargetSpv, targetLanguageVersion);

        std::string finalShaderSource = vsg::insertIncludes(vsg_shader->module->source, options);

        std::vector<std::string> combinedDefines(defines);
        for (auto& define : settings->defines) combinedDefines.push_back(define);
        if (!combinedDefines.empty()) finalShaderSource = combineSourceAndDefines(finalShaderSource, combinedDefines);

        vsg::debug("ShaderCompiler::compile() combinedDefines = ", combinedDefines);

        const char* str = finalShaderSource.c_str();
        shader->setStrings(&str, 1);

        EShMessages messages = EShMsgDefault;
        if (settings->generateDebugInfo)
        {
            messages = static_cast<EShMessages>(messages | EShMsgDebugInfo);
        }
        bool parseResult = shader->parse(builtInResources, settings->defaultVersion, settings->forwardCompatible, messages);

        if (parseResult)
        {
            debug("Successful compile\n", debugFormatShaderSource(finalShaderSource), "\n");

            program->addShader(shader);
            stageShaderMap[envStage] = vsg_shader;
        }
        else
        {
            // print error information
            warn("\n----  ", getFriendlyNameForShader(vsg_shader), "  ---- \n");
            warn(debugFormatShaderSource(finalShaderSource));
            warn("GLSL source failed to parse.");
            warn("glslang info log:\n", shader->getInfoLog());
            info("glslang debug info log: \n", shader->getInfoDebugLog());
            warn("--------");
        }
    }

    if (stageShaderMap.empty() || stageShaderMap.size() != shaders.size())
    {
        debug("ShaderCompiler::compile(Shaders& shaders) stageShaderMap.size() != shaders.size()");
        return false;
    }

    EShMessages messages = EShMsgDefault;
    if (!program->link(messages))
    {
        warn("\n----  Program  ----\n");

        for (auto& vsg_shader : shaders)
        {
            warn("\n", getFriendlyNameForShader(vsg_shader), ":\n");
            warn(debugFormatShaderSource(vsg_shader->module->source));
        }

        warn("Program failed to link.");
        warn("glslang info log: ", program->getInfoLog());
        warn("glslang debug info log: ", program->getInfoDebugLog());
        warn("--------");

        return false;
    }

    for (int eshl_stage = 0; eshl_stage < EShLangCount; ++eshl_stage)
    {
        auto vsg_shader = stageShaderMap[(EShLanguage)eshl_stage];
        if (vsg_shader && program->getIntermediate((EShLanguage)eshl_stage))
        {
            auto settings = vsg_shader->module->hints ? vsg_shader->module->hints : defaults;

            spv::SpvBuildLogger logger;

            glslang::SpvOptions spvOptions;
            if (settings)
            {
                spvOptions.generateDebugInfo = settings->generateDebugInfo;
                if (spvOptions.generateDebugInfo)
                {
                    spvOptions.emitNonSemanticShaderDebugInfo = true;
                    spvOptions.emitNonSemanticShaderDebugSource = true;
                }
            }

            vsg_shader->module->code.clear();
            glslang::GlslangToSpv(*(program->getIntermediate((EShLanguage)eshl_stage)), vsg_shader->module->code, &logger, &spvOptions);

#    if VSG_SUPPORTS_ShaderOptimizer
            if (settings && optimizer && settings->optimize)
            {
                optimizer->SetTargetEnv(selectTargetEnv(*settings));
                optimizer->RegisterPerformancePasses(true);
                vsg::ShaderModule::SPIRV unoptimized(std::move(vsg_shader->module->code));
                bool success = optimizer->Run(unoptimized.data(), unoptimized.size(), &vsg_shader->module->code);
                if (!success)
                {
                    warn("Shader optimisation failed, reverting to unoptimised.");
                    vsg_shader->module->code = std::move(unoptimized);
                }
            }
#    endif
        }
    }

    return true;
}
#else
bool ShaderCompiler::compile(ShaderStages&, const std::vector<std::string>&, ref_ptr<const Options> /*options*/)
{
    warn("ShaderCompile::compile(..) not supported,");
    return false;
}
#endif

bool ShaderCompiler::compile(ref_ptr<ShaderStage> shaderStage, const std::vector<std::string>& defines, ref_ptr<const Options> options)
{
    ShaderStages stages;
    stages.emplace_back(shaderStage);

    return compile(stages, defines, options);
}

std::string ShaderCompiler::combineSourceAndDefines(const std::string& source, const std::vector<std::string>& defines)
{
    if (defines.empty()) return source;

    // trim leading spaces/tabs
    auto trimLeading = [](std::string& str) {
        size_t startpos = str.find_first_not_of(" \t");
        if (std::string::npos != startpos)
        {
            str = str.substr(startpos);
        }
    };

    // trim trailing spaces/tabs/newlines
    auto trimTrailing = [](std::string& str) {
        size_t endpos = str.find_last_not_of(" \t\r\n");
        if (endpos != std::string::npos)
        {
            str.resize(endpos + 1);
        }
    };

    // sanitize line by trimming leading and trailing characters
    auto sanitise = [&trimLeading, &trimTrailing](std::string& str) {
        trimLeading(str);
        trimTrailing(str);
    };

    // return true if str starts with match string
    auto startsWith = [](const std::string& str, const std::string& match) {
        return str.compare(0, match.length(), match) == 0;
    };

    // returns the string between the start and end character
    auto stringBetween = [](const std::string& str, const char& startChar, const char& endChar) {
        auto start = str.find_first_of(startChar);
        if (start == std::string::npos) return std::string();

        auto end = str.find_first_of(endChar, start);
        if (end == std::string::npos) return std::string();

        if ((end - start) - 1 == 0) return std::string();

        return str.substr(start + 1, (end - start) - 1);
    };

    // returns the string split into sanitized tokens
    auto split = [&sanitise](const std::string& str, const char& separator) {
        std::vector<std::string> elements;

        std::string::size_type prev_pos = 0, pos = 0;

        while ((pos = str.find(separator, pos)) != std::string::npos)
        {
            auto substring = str.substr(prev_pos, pos - prev_pos);
            elements.push_back(substring);
            sanitise(elements.back());
            prev_pos = ++pos;
        }

        elements.push_back(str.substr(prev_pos, pos - prev_pos));
        sanitise(elements.back());

        return elements;
    };

    auto addLine = [](std::ostringstream& ss, const std::string& line) {
        ss << line << "\n";
    };

    /*
     * Verify that all the shader compiler defines are of the form "name" or "name=value" and that
     * the set doesn't contain entries with the same name and different values. This check is only
     * a temporary stopgap until the shader compiler defines container is updated from a
     * std::set<std::string>, to a std::map<std::string,std::string> to more properly manage defines
     * with optional values.
     */
    std::set<std::string> uniqueDefines;
    for (auto nameValueDef : defines)
    {
        auto nameValueDefElems = split(nameValueDef, '=');
        // simple check for formatting issues
        if (nameValueDefElems.size() != 1 && nameValueDefElems.size() != 2)
        {
            throw Exception{"Error: Incorrectly formatted shader compile define. Acceptable formats are either a name or name=value."};
        }
        // verify that a valueless define or a name=value define with the same name has not already been encountered
        if (uniqueDefines.count(nameValueDefElems.front()) != 0)
        {
            throw Exception{"Error: Found two shader compiler defines with the same name and different values."};
        }
        // keep track that we have seen this define name
        uniqueDefines.insert(nameValueDefElems.front());
    }

    std::istringstream iss(source);
    std::ostringstream headerstream;
    std::ostringstream sourcestream;

    const std::string versionmatch = "#version";
    const std::string importdefinesmatch = "#pragma import_defines";

    std::vector<std::string> finaldefines;

    for (std::string line; std::getline(iss, line);)
    {
        std::string sanitisedline = line;
        sanitise(sanitisedline);

        // is it the version
        if (startsWith(sanitisedline, versionmatch))
        {
            addLine(headerstream, line);
        }
        // is it the defines import
        else if (startsWith(sanitisedline, importdefinesmatch))
        {
            // get the import defines between ()
            auto csv = stringBetween(sanitisedline, '(', ')');
            auto importedDefines = split(csv, ',');

            addLine(headerstream, line);

            // loop over the requested defines and see if it's also in the imported defines, if so insert a define line
            for (auto nameValueDef : defines)
            {
                // separate the define name from its optional value
                auto nameValueDefElems = split(nameValueDef, '=');
                assert(nameValueDefElems.size() == 1 || nameValueDefElems.size() == 2);

                auto finditr = std::find(importedDefines.begin(), importedDefines.end(), nameValueDefElems.front());
                if (finditr != defines.end())
                {
                    // output the pre-processor define directive with its name and optional value
                    addLine(headerstream, "#define " + nameValueDefElems.front() +
                                (nameValueDefElems.size() == 2
                                     ? " "+nameValueDefElems.back() : std::string()));
                }
            }
        }
        else
        {
            // standard source line
            addLine(sourcestream, line);
        }
    }

    return headerstream.str() + sourcestream.str();
}

void ShaderCompiler::apply(Node& node)
{
    node.traverse(*this);
}

void ShaderCompiler::apply(BindGraphicsPipeline& bgp)
{
    auto pipeline = bgp.pipeline;
    if (!pipeline) return;

    // compile shaders if required
    bool requiresShaderCompiler = false;
    for (const auto& shaderStage : pipeline->stages)
    {
        if (shaderStage->module)
        {
            if (shaderStage->module->code.empty() && !(shaderStage->module->source.empty()))
            {
                requiresShaderCompiler = true;
            }
        }
    }

    if (requiresShaderCompiler)
    {
        compile(pipeline->stages); // may need to map defines and paths in some fashion
    }
}

void ShaderCompiler::apply(BindComputePipeline& bcp)
{
    auto pipeline = bcp.pipeline;
    if (!pipeline) return;

    auto stage = pipeline->stage;

    // compile shaders if required
    bool requiresShaderCompiler = stage && stage->module && stage->module->code.empty() && !(stage->module->source.empty());

    if (requiresShaderCompiler)
    {
        compile(stage); // may need to map defines and paths in some fashion
    }
}

void ShaderCompiler::apply(BindRayTracingPipeline& brtp)
{
    auto pipeline = brtp.getPipeline();
    if (!pipeline) return;

    // compile shaders if required
    bool requiresShaderCompiler = false;
    for (const auto& shaderStage : pipeline->getShaderStages())
    {
        if (shaderStage->module)
        {
            if (shaderStage->module->code.empty() && !(shaderStage->module->source.empty()))
            {
                requiresShaderCompiler = true;
            }
        }
    }

    if (requiresShaderCompiler)
    {
        compile(pipeline->getShaderStages()); // may need to map defines and paths in some fashion
    }
}
