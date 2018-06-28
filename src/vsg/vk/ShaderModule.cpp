#include <vsg/vk/ShaderModule.h>

namespace vsg
{

ShaderModule::ShaderModule(Device* device, VkShaderModule shaderModule, VkAllocationCallbacks* pAllocator):
    _device(device),
    _shaderModule(shaderModule),
    _pAllocator(pAllocator)
{
}

ShaderModule::~ShaderModule()
{
    if (_shaderModule)
    {
        std::cout<<"Calling vkDestroyShaderModule()"<<std::endl;
        vkDestroyShaderModule(*_device, _shaderModule, _pAllocator);
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