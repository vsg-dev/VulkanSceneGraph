#include <vsg/vk/ShaderModule.h>

namespace vsg
{

ShaderModule::ShaderModule(VkShaderStageFlagBits stage, const std::string& entryPointName, VkShaderModule shaderModule, Device* device, AllocationCallbacks* allocator):
    _shaderModule(shaderModule),
    _stage(stage),
    _name(entryPointName),
    _device(device),
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

ShaderModule::Result ShaderModule::read(Device* device, VkShaderStageFlagBits stage, const std::string& entryPointName, const std::string& filename, AllocationCallbacks* allocator)
{
    std::vector<char> buffer;
    if (readFile(buffer, filename))
    {
        return ShaderModule::create(device, stage, entryPointName, buffer, allocator);
    }
    else
    {
        return ShaderModule::Result("Error: vsg::ShaderModule::read(..) failed to read shader file.", VK_INCOMPLETE);
    }
}

}
