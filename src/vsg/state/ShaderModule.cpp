/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/io/read.h>
#include <vsg/state/ShaderModule.h>
#include <vsg/vk/Context.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ShaderCompileSettings
//
int ShaderCompileSettings::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(vulkanVersion, rhs.vulkanVersion))) return result;
    if ((result = compare_value(clientInputVersion, rhs.clientInputVersion))) return result;
    if ((result = compare_value(language, rhs.language))) return result;
    if ((result = compare_value(defaultVersion, rhs.defaultVersion))) return result;
    if ((result = compare_value(target, rhs.target))) return result;
    if ((result = compare_value(forwardCompatible, rhs.forwardCompatible))) return result;
    if ((result = compare_value(generateDebugInfo, rhs.generateDebugInfo))) return result;
    return compare_container(defines, rhs.defines);
}

void ShaderCompileSettings::read(Input& input)
{
    input.read("vulkanVersion", vulkanVersion);
    input.read("clientInputVersion", clientInputVersion);
    input.readValue<int>("language", language);
    input.read("defaultVersion", defaultVersion);
    input.readValue<int>("target", target);
    input.read("forwardCompatible", forwardCompatible);

    if (input.version_greater_equal(1, 0, 4))
    {
        input.read("generateDebugInfo", generateDebugInfo);
    }

    input.readValues("defines", defines);
}

void ShaderCompileSettings::write(Output& output) const
{
    output.write("vulkanVersion", vulkanVersion);
    output.write("clientInputVersion", clientInputVersion);
    output.writeValue<int>("language", language);
    output.write("defaultVersion", defaultVersion);
    output.writeValue<int>("target", target);
    output.write("forwardCompatible", forwardCompatible);

    if (output.version_greater_equal(1, 0, 4))
    {
        output.write("generateDebugInfo", generateDebugInfo);
    }

    output.writeValues("defines", defines);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Shader
//
ShaderModule::ShaderModule()
{
}

ShaderModule::ShaderModule(const std::string& in_source, ref_ptr<ShaderCompileSettings> in_hints) :
    source(in_source),
    hints(in_hints)
{
}

ShaderModule::ShaderModule(const SPIRV& in_code) :
    code(in_code)
{
}

ShaderModule::ShaderModule(const std::string& in_source, const SPIRV& in_code) :
    source(in_source),
    code(in_code)
{
}

ShaderModule::~ShaderModule()
{
}

int ShaderModule::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(source, rhs.source))) return result;
    if ((result = compare_pointer(hints, rhs.hints))) return result;
    return compare_value(code, rhs.code);
}

void ShaderModule::read(Input& input)
{
    Object::read(input);

    input.readObject("hints", hints);
    input.read("source", source);
    code.resize(input.readValue<uint32_t>("code"));
    input.read(code.size(), code.data());
}

void ShaderModule::write(Output& output) const
{
    Object::write(output);

    output.writeObject("hints", hints);
    output.write("source", source);
    output.writeValue<uint32_t>("code", code.size());
    output.writePropertyName(""); // convenient way of forcing an indent to the appropriate column when writing out to ascii, doesn't require a matching readPropertyName() in ShaderModule::read(..).
    output.write(code.size(), code.data());
    output.writeEndOfLine();
}

void ShaderModule::compile(Context& context)
{
    if (!_implementation[context.deviceID]) _implementation[context.deviceID] = Implementation::create(context.device, this);
}

ShaderModule::Implementation::Implementation(Device* device, ShaderModule* shaderModule) :
    _device(device)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.codeSize = shaderModule->code.size() * sizeof(ShaderModule::SPIRV::value_type);
    createInfo.pCode = shaderModule->code.data();

    if (VkResult result = vkCreateShaderModule(*device, &createInfo, _device->getAllocationCallbacks(), &_shaderModule); result != VK_SUCCESS)
    {
        throw Exception{"Error: vsg::ShaderModule::create(...) failed to create shader module.", result};
    }
}

ShaderModule::Implementation::~Implementation()
{
    vkDestroyShaderModule(*_device, _shaderModule, _device->getAllocationCallbacks());
}

std::string vsg::insertIncludes(const std::string& source, ref_ptr<const Options> options)
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

            // check for include part of #pragma include usage
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

        auto includedSource = vsg::read_cast<ShaderModule>(filename, options);
        if (includedSource)
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

            code.insert(pos, includedSource->source);
            pos += includedSource->source.size();

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
