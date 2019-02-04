/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/ShaderModule.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Shader
//

Shader::Shader(VkShaderStageFlagBits stage, const std::string& entryPointName, const SPIRV& spirv) :
    _stage(stage),
    _entryPointName(entryPointName),
    _spirv(spirv)
{
}

Shader::Shader(VkShaderStageFlagBits stage, const std::string& entryPointName, const Source& source) :
    _stage(stage),
    _entryPointName(entryPointName),
    _source(source)
{
}

Shader::Shader(VkShaderStageFlagBits stage, const std::string& entryPointName, const Source& source, const SPIRV& spirv) :
    _stage(stage),
    _entryPointName(entryPointName),
    _source(source),
    _spirv(spirv)
{
}

Shader::~Shader()
{
}

Shader::Result Shader::read(VkShaderStageFlagBits stage, const std::string& entryPointName, const std::string& filename)
{
    SPIRV buffer;
    if (readFile(buffer, filename))
    {
        return Result(new Shader(stage, entryPointName, buffer));
    }
    else
    {
        return Shader::Result("Error: vsg::Shader::read(..) failed to read shader file.", VK_INCOMPLETE);
    }
}


void Shader::read(Input& input)
{
    Object::read(input);

    _stage = static_cast<VkShaderStageFlagBits>(input.readValue<int32_t>("Stage"));

    input.read("EntryPoint", _entryPointName);
    input.read("Source", _source);

    _spirv.resize(input.readValue<uint32_t>("SPIRVSize"));
    input.matchPropertyName("SPIRV");
    input.read(_spirv.size(), _spirv.data());
}

void Shader::write(Output& output) const
{
    Object::write(output);

    output.writeValue<int32_t>("Stage", _stage);

    output.write("EntryPoint",  _entryPointName);
    output.write("Source", _source);

    output.writeValue<uint32_t>("SPIRVSize", _spirv.size());
    output.writePropertyName("SPIRV");
    output.write(_spirv.size(), _spirv.data());
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ShaderModule
//
ShaderModule::ShaderModule(VkShaderModule shaderModule, Device* device, Shader* shader, AllocationCallbacks* allocator) :
    _shaderModule(shaderModule),
    _device(device),
    _shader(shader),
    _allocator(allocator)
{
}

ShaderModule::~ShaderModule()
{
    if (_shaderModule)
    {
        vkDestroyShaderModule(*_device, _shaderModule, _allocator);
    }
}

ShaderModule::Result ShaderModule::create(Device* device, Shader* shader, AllocationCallbacks* allocator)
{
    if (!device || !shader)
    {
        return Result("Error: vsg::ShaderModule::create(...) failed, requires valid logical device and Shader.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    if (shader->spirv().empty())
    {
        return Result("Error: vsg::ShaderModule::create(...) failed. requires Shader with valid spirv contents.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shader->spirv().size() * sizeof(Shader::SPIRV::value_type);
    createInfo.pCode = shader->spirv().data();

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(*device, &createInfo, allocator, &shaderModule);
    if (result == VK_SUCCESS)
    {
        return Result(new ShaderModule(shaderModule, device, shader, allocator));
    }
    else
    {
        return Result("Error: vsg::ShaderModule::create(...) failed to create shader module.", result);
    }
}
