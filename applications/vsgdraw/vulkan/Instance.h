#pragma once

#include <vsg/core/ref_ptr.h>

#include <vulkan/vulkan.h>

#include <set>
#include <vector>
#include <iostream>

/////////////////////////////////////////////////////////////////////
//
// start of vulkan code
//
namespace vsg
{
    using Names = std::vector<const char*>;

    Names validateInstancelayerNames(const Names& names)
    {
        if (names.empty()) return names;

        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        using NameSet = std::set<std::string>;
        NameSet layerNames;
        for (const auto& layer : availableLayers)
        {
            std::cout<<"layer="<<layer.layerName<<std::endl;
            if (layer.layerName) layerNames.insert(layer.layerName);
        }

        Names validatedNames;
        validatedNames.reserve(names.size());
        for (const auto& requestedName : names)
        {
            if (layerNames.count(requestedName)!=0)
            {
                std::cout<<"valid requested layer : "<<requestedName<<std::endl;
                validatedNames.push_back(requestedName);
            }
            else
            {
                std::cout<<"Warning : requested invalid layer : "<<requestedName<<std::endl;
            }
        }

        return validatedNames;
    }

    VkQueue createDeviceQueue(VkDevice device, int graphicsFamily)
    {
        VkQueue queue;
        vkGetDeviceQueue(device, graphicsFamily, 0, &queue);
        return queue;
    }

    class Instance : public vsg::Object
    {
    public:
        Instance(VkInstance instance, VkAllocationCallbacks* pAllocator=nullptr) : _instance(instance), _pAllocator(pAllocator) {}

        Instance(Names& instanceExtensions, Names& layers, VkAllocationCallbacks* pAllocator=nullptr):
            _instance(VK_NULL_HANDLE),
            _pAllocator(pAllocator)
        {
            // applictin info
            VkApplicationInfo appInfo = {};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Test";
            appInfo.pEngineName = "VulkanSceneGraph";
            appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            VkInstanceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            createInfo.enabledExtensionCount = instanceExtensions.size();
            createInfo.ppEnabledExtensionNames = instanceExtensions.empty() ? nullptr : instanceExtensions.data();

            createInfo.enabledLayerCount = layers.size();
            createInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

            if (vkCreateInstance(&createInfo, nullptr, &_instance) == VK_SUCCESS)
            {
                std::cout<<"Created VkInstance"<<std::endl;
            }
            else
            {
                std::cout<<"Failed to create VkInstance"<<std::endl;
            }
        }

        operator VkInstance() const { return _instance; }

    protected:
        virtual ~Instance()
        {
            if (_instance)
            {
                std::cout<<"Calling vkDestroyInstance"<<std::endl;
                vkDestroyInstance(_instance, _pAllocator);
            }
        }

        VkInstance              _instance;
        VkAllocationCallbacks*  _pAllocator;
    };
}
