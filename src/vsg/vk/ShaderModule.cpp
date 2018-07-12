#include <vsg/vk/ShaderModule.h>

namespace vsg
{

ShaderModule::ShaderModule(Device* device, VkShaderStageFlagBits stage, const std::string& entryPointName, VkShaderModule shaderModule, AllocationCallbacks* allocator):
    _device(device),
    _shaderModule(shaderModule),
    _stage(stage),
    _name(entryPointName),
    _allocator(allocator)
{
}

ShaderModule::~ShaderModule()
{
    if (_shaderModule)
    {
        std::cout<<"Calling vkDestroyShaderModule()"<<std::endl;
        vkDestroyShaderModule(*_device, _shaderModule, *_allocator);
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