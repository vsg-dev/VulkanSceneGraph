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
        ShaderModule(VkShaderStageFlagBits stage, const std::string& entryPointName, VkShaderModule shaderModule, Device* device, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<ShaderModule, VkResult, VK_SUCCESS>;

        template<typename T>
        static Result create(Device* device, VkShaderStageFlagBits stage, const std::string& entryPointName, const T& shader, AllocationCallbacks* allocator=nullptr)
        {
            if (!device)
            {
                return Result("Error: vsg::ShaderModule::create(...) failed to create logical device, undefined Instance.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
            }

            VkShaderModuleCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = shader.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.data());

            VkShaderModule shaderModule;
            VkResult result = vkCreateShaderModule(*device, &createInfo, allocator, &shaderModule);
            if (result == VK_SUCCESS)
            {
                return new ShaderModule(stage, entryPointName, shaderModule, device, allocator);
            }
            else
            {
                return Result("Error: vsg::ShaderModule::create(...) failed to create shader module.", result);
            }
        }

        static Result read(Device* device, VkShaderStageFlagBits stage, const std::string& entryPointName, const std::string& filename, AllocationCallbacks* allocator=nullptr);

        void setStage(VkShaderStageFlagBits stage) { _stage = stage; }
        VkShaderStageFlagBits getStage() const { return _stage; }

        void setEntryPointName(const std::string& name) { _name = name; }
        const std::string& getEntryPointName() const { return _name; }

        // add Shader type ? VK_SHADER_STAGE_VERTEX_BIT vs VK_SHADER_STAGE_FRAGMENT_BIT etc.
        // so that it can be used to set up VkPipelineShaderStageCreateInfo

        operator VkShaderModule () const { return _shaderModule; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~ShaderModule();

        VkShaderModule                  _shaderModule;
        VkShaderStageFlagBits           _stage;
        std::string                     _name;

        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

    using ShaderModules = std::vector<ref_ptr<ShaderModule>>;

}
