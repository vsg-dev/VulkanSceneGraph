#pragma once

#include <vsg/vk/Device.h>
#include <fstream>
#include <iostream>

namespace vsg
{
    template<typename T>
    bool readFile(T& buffer, const std::string& filename)
    {
        std::ifstream fin(filename, std::ios::ate | std::ios::binary);
        if (!fin.is_open()) return false;

        size_t fileSize = fin.tellg();

        using value_type = typename T::value_type;
        size_t valueSize = sizeof(value_type);
        buffer.resize(fileSize/valueSize);

        fin.seekg(0);
        fin.read(buffer.data(), buffer.size()*valueSize);
        fin.close();

        return true;
    }

    class ShaderModule : public vsg::Object
    {
    public:
        ShaderModule(Device* device, VkShaderModule shaderModule, VkAllocationCallbacks* pAllocator=nullptr);

        template<typename T>
        ShaderModule(Device* device, const T& shader, VkAllocationCallbacks* pAllocator=nullptr):
            _device(device),
            _shaderModule(VK_NULL_HANDLE),
            _pAllocator(pAllocator)
        {
            VkShaderModuleCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = shader.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.data());

            if (vkCreateShaderModule(*device, &createInfo, nullptr, &_shaderModule) != VK_SUCCESS)
            {
                std::cout<<"Failed to create shader module"<<std::endl;
            }
        }

        operator VkShaderModule () const { return _shaderModule; }

    protected:
        virtual ~ShaderModule();

        ref_ptr<Device>         _device;
        VkShaderModule          _shaderModule;
        VkAllocationCallbacks*  _pAllocator;
    };


    extern vsg::ref_ptr<ShaderModule> readShaderModule(Device* device, const std::string& filename);
}
