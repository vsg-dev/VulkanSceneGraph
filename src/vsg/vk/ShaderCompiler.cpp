/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/raytracing/RayTracingPipeline.h>
#include <vsg/state/ComputePipeline.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/StateGroup.h>
#include <vsg/vk/ShaderCompiler.h>

#ifdef HAS_GLSLANG
#    include <glslang/Public/ShaderLang.h>
#    include <glslang/SPIRV/GlslangToSpv.h>

#    include "ResourceLimits.cpp"
#    include "ResourceLimits.h"
#endif

#include <algorithm>
#include <iomanip>
#include <iostream>


#define HAS_NV_RAYTRACNG (VK_HEADER_VERSION>=92)

using namespace vsg;

#if 1
#    define DEBUG_OUTPUT std::cout
#else
#    define DEBUG_OUTPUT \
        if (false) std::cout
#endif
#define INFO_OUTPUT std::cout

#ifdef HAS_GLSLANG
static std::atomic_uint s_intialized = 0;

static void s_initializeProcess()
{
    if (s_intialized.fetch_add(1) == 0)
    {
        glslang::InitializeProcess();
    }
}

static void s_finalizeProcess()
{
    if (s_intialized.fetch_sub(1) == 1)
    {
        glslang::FinalizeProcess();
    }
}
#endif

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
    Inherit()
{
}

ShaderCompiler::~ShaderCompiler()
{
#ifdef HAS_GLSLANG
    s_finalizeProcess();
#endif
}

#ifdef HAS_GLSLANG
bool ShaderCompiler::compile(ShaderStages& shaders, const std::vector<std::string>& defines, const Paths& paths)
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
#    ifdef HAS_NV_RAYTRACNG
        case (VK_SHADER_STAGE_RAYGEN_BIT_NV): return "RayGen Shader";
        case (VK_SHADER_STAGE_ANY_HIT_BIT_NV): return "Any Hit Shader";
        case (VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV): return "Closest Hit Shader";
        case (VK_SHADER_STAGE_MISS_BIT_NV): return "Miss Shader";
        case (VK_SHADER_STAGE_INTERSECTION_BIT_NV): return "Intersection Shader";
        case (VK_SHADER_STAGE_CALLABLE_BIT_NV): return "Callable Shader";
        case (VK_SHADER_STAGE_TASK_BIT_NV): return "Task Shader";
        case (VK_SHADER_STAGE_MESH_BIT_NV): return "Mesh Shader";
#    endif
        default: return "Unkown Shader Type";
        }
        return "";
    };

    using StageShaderMap = std::map<EShLanguage, ref_ptr<ShaderStage>>;
    using TShaders = std::list<std::unique_ptr<glslang::TShader>>;
    TShaders tshaders;

    TBuiltInResource builtInResources = glslang::DefaultTBuiltInResource;

    StageShaderMap stageShaderMap;
    std::unique_ptr<glslang::TProgram> program(new glslang::TProgram);

    for (auto& vsg_shader : shaders)
    {
        EShLanguage envStage = EShLangCount;
        switch (vsg_shader->stage)
        {
        case (VK_SHADER_STAGE_VERTEX_BIT): envStage = EShLangVertex; break;
        case (VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT): envStage = EShLangTessControl; break;
        case (VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT): envStage = EShLangTessEvaluation; break;
        case (VK_SHADER_STAGE_GEOMETRY_BIT): envStage = EShLangGeometry; break;
        case (VK_SHADER_STAGE_FRAGMENT_BIT): envStage = EShLangFragment; break;
        case (VK_SHADER_STAGE_COMPUTE_BIT): envStage = EShLangCompute; break;
#    ifdef HAS_NV_RAYTRACNG
        case (VK_SHADER_STAGE_RAYGEN_BIT_NV): envStage = EShLangRayGenNV; break;
        case (VK_SHADER_STAGE_ANY_HIT_BIT_NV): envStage = EShLangAnyHitNV; break;
        case (VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV): envStage = EShLangClosestHitNV; break;
        case (VK_SHADER_STAGE_MISS_BIT_NV): envStage = EShLangMissNV; break;
        case (VK_SHADER_STAGE_INTERSECTION_BIT_NV): envStage = EShLangIntersectNV; break;
        case (VK_SHADER_STAGE_CALLABLE_BIT_NV): envStage = EShLangCallableNV; break;
        case (VK_SHADER_STAGE_TASK_BIT_NV): envStage = EShLangTaskNV; break;
        case (VK_SHADER_STAGE_MESH_BIT_NV): envStage = EShLangMeshNV; break;
#    endif
        default: break;
        }

        if (envStage == EShLangCount)
        {
            INFO_OUTPUT<<" Warning ShaderCompiler::compile() unsupported stage : " <<vsg_shader->stage<<std::endl;
            return false;
        }

        glslang::TShader* shader(new glslang::TShader(envStage));
        tshaders.emplace_back(shader);

        shader->setEnvInput(glslang::EShSourceGlsl, envStage, glslang::EShClientVulkan, 150);
        shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
        shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

        std::string shaderSourceWithIncludesInserted = insertIncludes(vsg_shader->module->source, paths);
        std::string finalShaderSource = combineSourceAndDefines(shaderSourceWithIncludesInserted, defines);

        const char* str = finalShaderSource.c_str();
        shader->setStrings(&str, 1);

        int defaultVersion = 110; // 110 desktop, 100 non desktop
        bool forwardCompatible = false;
        EShMessages messages = EShMsgDefault;
        bool parseResult = shader->parse(&builtInResources, defaultVersion, forwardCompatible, messages);

        if (parseResult)
        {
            program->addShader(shader);

            stageShaderMap[envStage] = vsg_shader;
        }
        else
        {
            // print error infomation
            INFO_OUTPUT << std::endl
                        << "----  " << getFriendlyNameForShader(vsg_shader) << "  ----" << std::endl
                        << std::endl;
            INFO_OUTPUT << debugFormatShaderSource(vsg_shader->module->source) << std::endl;
            INFO_OUTPUT << "Warning: GLSL source failed to parse." << std::endl;
            INFO_OUTPUT << "glslang info log: " << std::endl
                        << shader->getInfoLog();
            DEBUG_OUTPUT << "glslang debug info log: " << std::endl
                         << shader->getInfoDebugLog();
            INFO_OUTPUT << "--------" << std::endl;
        }
    }

    if (stageShaderMap.empty() || stageShaderMap.size() != shaders.size())
    {
        DEBUG_OUTPUT << "ShaderCompiler::compile(Shaders& shaders) stageShaderMap.size() != shaders.size()" << std::endl;
        return false;
    }

    EShMessages messages = EShMsgDefault;
    if (!program->link(messages))
    {
        INFO_OUTPUT << std::endl
                    << "----  Program  ----" << std::endl
                    << std::endl;

        for (auto& vsg_shader : shaders)
        {
            INFO_OUTPUT << std::endl
                        << getFriendlyNameForShader(vsg_shader) << ":" << std::endl
                        << std::endl;
            INFO_OUTPUT << debugFormatShaderSource(vsg_shader->module->source) << std::endl;
        }

        INFO_OUTPUT << "Warning: Program failed to link." << std::endl;
        INFO_OUTPUT << "glslang info log: " << std::endl
                    << program->getInfoLog();
        DEBUG_OUTPUT << "glslang debug info log: " << std::endl
                     << program->getInfoDebugLog();
        INFO_OUTPUT << "--------" << std::endl;

        return false;
    }

    for (int eshl_stage = 0; eshl_stage < EShLangCount; ++eshl_stage)
    {
        auto vsg_shader = stageShaderMap[(EShLanguage)eshl_stage];
        if (vsg_shader && program->getIntermediate((EShLanguage)eshl_stage))
        {
            ShaderModule::SPIRV spirv;
            std::string warningsErrors;
            spv::SpvBuildLogger logger;
            glslang::SpvOptions spvOptions;
            glslang::GlslangToSpv(*(program->getIntermediate((EShLanguage)eshl_stage)), vsg_shader->module->code, &logger, &spvOptions);
        }
    }

    return true;
}
#else
bool ShaderCompiler::compile(ShaderStages&, const std::vector<std::string>&, const Paths&)
{
    std::cout << "ShaderCompile::compile(..) not supported," << std::endl;
    return false;
}
#endif

bool ShaderCompiler::compile(ref_ptr<ShaderStage> shaderStage, const std::vector<std::string>& defines, const Paths& paths)
{
    ShaderStages stages;
    stages.emplace_back(shaderStage);

    return compile(stages, defines, paths);
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
        size_t endpos = str.find_last_not_of(" \t\n");
        if (endpos != std::string::npos)
        {
            str = str.substr(0, endpos + 1);
        }
    };

    // sanitise line by triming leading and trailing characters
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

    auto split = [](const std::string& str, const char& seperator) {
        std::vector<std::string> elements;

        std::string::size_type prev_pos = 0, pos = 0;

        while ((pos = str.find(seperator, pos)) != std::string::npos)
        {
            auto substring = str.substr(prev_pos, pos - prev_pos);
            elements.push_back(substring);
            prev_pos = ++pos;
        }

        elements.push_back(str.substr(prev_pos, pos - prev_pos));

        return elements;
    };

    auto addLine = [](std::ostringstream& ss, const std::string& line) {
        ss << line << "\n";
    };

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

            // loop the imported defines and see if it's also requested in defines, if so insert a define line
            for (auto importedDef : importedDefines)
            {
                auto sanitiesedImportDef = importedDef;
                sanitise(sanitiesedImportDef);

                auto finditr = std::find(defines.begin(), defines.end(), sanitiesedImportDef);
                if (finditr != defines.end())
                {
                    addLine(headerstream, "#define " + sanitiesedImportDef);
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

std::string ShaderCompiler::insertIncludes(const std::string& source, const Paths& paths)
{
    std::string code = source;
    std::string startOfIncludeMarker("// Start of include code : ");
    std::string endOfIncludeMarker("// End of include code : ");
    std::string failedLoadMarker("// Failed to load include code : ");

#if defined(__APPLE__)
    std::string endOfLine("\r");
#elif defined(_WIN32)
    std::string endOfLine("\r\n");
#else
    std::string endOfLine("\n");
#endif

    std::string::size_type pos = 0;
    std::string::size_type pragma_pos = 0;
    std::string::size_type include_pos = 0;
    while ((pos != std::string::npos) && (((pragma_pos = code.find("#pragma", pos)) != std::string::npos) || (include_pos = code.find("#include", pos)) != std::string::npos))
    {
        pos = (pragma_pos != std::string::npos) ? pragma_pos : include_pos;

        std::string::size_type start_of_pragma_line = pos;
        std::string::size_type end_of_line = code.find_first_of("\n\r", pos);

        if (pragma_pos != std::string::npos)
        {
            // we have #pragma usage so skip to the start of the first non white space
            pos = code.find_first_not_of(" \t", pos + 7);
            if (pos == std::string::npos) break;

            // check for include part of #pragma imclude usage
            if (code.compare(pos, 7, "include") != 0)
            {
                pos = end_of_line;
                continue;
            }

            // found include entry so skip to next non white space
            pos = code.find_first_not_of(" \t", pos + 7);
            if (pos == std::string::npos) break;
        }
        else
        {
            // we have #include usage so skip to next non white space
            pos = code.find_first_not_of(" \t", pos + 8);
            if (pos == std::string::npos) break;
        }

        std::string::size_type num_characters = (end_of_line == std::string::npos) ? code.size() - pos : end_of_line - pos;
        if (num_characters == 0) continue;

        // prune trailing white space
        while (num_characters > 0 && (code[pos + num_characters - 1] == ' ' || code[pos + num_characters - 1] == '\t')) --num_characters;

        if (code[pos] == '\"')
        {
            if (code[pos + num_characters - 1] != '\"')
            {
                num_characters -= 1;
            }
            else
            {
                num_characters -= 2;
            }

            ++pos;
        }

        std::string filename(code, pos, num_characters);

        code.erase(start_of_pragma_line, (end_of_line == std::string::npos) ? code.size() - start_of_pragma_line : end_of_line - start_of_pragma_line);
        pos = start_of_pragma_line;

        std::string includedSource = readShaderSource(filename, paths);

        if (!includedSource.empty())
        {
            if (!startOfIncludeMarker.empty())
            {
                code.insert(pos, startOfIncludeMarker);
                pos += startOfIncludeMarker.size();
                code.insert(pos, filename);
                pos += filename.size();
                code.insert(pos, endOfLine);
                pos += endOfLine.size();
            }

            code.insert(pos, includedSource);
            pos += includedSource.size();

            if (!endOfIncludeMarker.empty())
            {
                code.insert(pos, endOfIncludeMarker);
                pos += endOfIncludeMarker.size();
                code.insert(pos, filename);
                pos += filename.size();
                code.insert(pos, endOfLine);
                pos += endOfLine.size();
            }
        }
        else
        {
            if (!failedLoadMarker.empty())
            {
                code.insert(pos, failedLoadMarker);
                pos += failedLoadMarker.size();
                code.insert(pos, filename);
                pos += filename.size();
                code.insert(pos, endOfLine);
                pos += endOfLine.size();
            }
        }
    }
    return code;
}

std::string ShaderCompiler::readShaderSource(const Path& filename, const Paths& paths)
{
    Path foundFile = findFile(filename, paths);
    if (foundFile.empty()) return std::string();

    std::ifstream fin(foundFile);
    if (!fin) return std::string();

    size_t fileSize = fin.tellg();
    std::string source;
    source.resize(fileSize);

    fin.seekg(0);
    fin.read(reinterpret_cast<char*>(source.data()), fileSize);
    fin.close();

    return source;
}

void ShaderCompiler::apply(Node& node)
{
    node.traverse(*this);
}

void ShaderCompiler::apply(StateGroup& stategroup)
{
    for (auto& stateCommands : stategroup.getStateCommands())
    {
        stateCommands->accept(*this);
    }

    stategroup.traverse(*this);
}

void ShaderCompiler::apply(BindGraphicsPipeline& bgp)
{
    auto pipeline = bgp.pipeline;
    if (!pipeline) return;

    // compile shaders if required
    bool requiresShaderCompiler = false;
    for (auto& shaderStage : pipeline->stages)
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
    for (auto& shaderStage : pipeline->getShaderStages())
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
