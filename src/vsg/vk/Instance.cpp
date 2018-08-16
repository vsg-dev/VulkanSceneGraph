#include <vsg/vk/Instance.h>

#include <set>
#include <iostream>

namespace vsg
{

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

Instance::Instance(VkInstance instance, AllocationCallbacks* allocator) :
    _instance(instance),
    _allocator(allocator)
{
}

Instance::~Instance()
{
    if (_instance)
    {
        std::cout<<"Calling vkDestroyInstance"<<std::endl;
        vkDestroyInstance(_instance, _allocator);
    }
}

Instance::Result Instance::create(Names& instanceExtensions, Names& layers, AllocationCallbacks* allocator)
{
    std::cout<<"Instance::create()"<<std::endl;
    std::cout<<"instanceExtensions : "<<std::endl;
    for(auto & name : instanceExtensions)
    {
        std::cout<<"    "<<name<<std::endl;
    }

    std::cout<<"layers : "<<std::endl;
    for(auto & name : layers)
    {
        std::cout<<"    "<<name<<std::endl;
    }

    VkAllocationCallbacks* ac = (allocator != nullptr) ? allocator : nullptr;
    std::cout<<"allocator : "<<allocator<<std::endl;
    std::cout<<"VkAllocationCallbacks* : "<<ac<<std::endl;

    ac = nullptr;

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


    VkInstance instance;
    VkResult result = vkCreateInstance(&createInfo, allocator, &instance);
    if (result == VK_SUCCESS)
    {
        std::cout<<"Created VkInstance"<<std::endl;
        return new Instance(instance, allocator);
    }
    else
    {
        return Result("Error: vsg::Instance::create(...) failed to create VkInstance.", result);
    }
}

} // end of namespace vsg
