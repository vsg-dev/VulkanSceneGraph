#include <vsg/vk/ShaderModule.h>

namespace vsg
{

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
        return new Shader(stage, entryPointName, buffer);
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
        return new ShaderModule(shaderModule, device, shader, allocator);
    }
    else
    {
        return Result("Error: vsg::ShaderModule::create(...) failed to create shader module.", result);
    }
}

}
