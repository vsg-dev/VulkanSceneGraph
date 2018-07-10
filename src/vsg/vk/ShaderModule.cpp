#include <vsg/vk/ShaderModule.h>

namespace vsg
{

ShaderModule::ShaderModule(Device* device, VkShaderModule shaderModule, AllocationCallbacks* allocator):
    _device(device),
    _shaderModule(shaderModule),
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

vsg::ref_ptr<ShaderModule> readShaderModule(Device* device, const std::string& filename)
{
    std::vector<char> buffer;
    if (readFile(buffer, filename))
    {
        return new ShaderModule(device, buffer);
    }
    else
    {
        return nullptr;
    }
}

}