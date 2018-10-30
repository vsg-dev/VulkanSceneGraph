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

Shader::Shader(VkShaderStageFlagBits stage, const std::string& entryPointName, const Contents& contents) :
    _stage(stage),
    _entryPointName(entryPointName),
    _contents(contents)
{
}

Shader::~Shader()
{
}

Shader::Result Shader::read(VkShaderStageFlagBits stage, const std::string& entryPointName, const std::string& filename)
{
    Contents buffer;
    if (readFile(buffer, filename))
    {
        return Result(new Shader(stage, entryPointName, buffer));
    }
    else
    {
        return Shader::Result("Error: vsg::Shader::read(..) failed to read shader file.", VK_INCOMPLETE);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ShaderModule
//
ShaderModule::ShaderModule(VkShaderModule shaderModule, Device* device, Shader* shader, AllocationCallbacks* allocator):
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
        return Result("Error: vsg::ShaderModule::create(...) failed to create logical device, or Shader.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shader->contents().size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shader->contents().data());

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
