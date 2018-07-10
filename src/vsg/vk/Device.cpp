#include <vsg/vk/Device.h>

#include <set>
#include <iostream>

namespace vsg
{

Device::Device(Instance* instance, VkDevice device, AllocationCallbacks* allocator) :
    _instance(instance),
    _device(device),
    _allocator(allocator)
{
}

Device::Device(Instance* instance, PhysicalDevice* physicalDevice, Names& layers, Names& deviceExtensions, AllocationCallbacks* allocator) :
    _instance(instance),
    _physicalDevice(physicalDevice),
    _device(VK_NULL_HANDLE),
    _allocator(allocator)
{
    std::set<int> uniqueQueueFamiles = {physicalDevice->getGraphicsFamily(), physicalDevice->getPresentFamily()};
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    for (int queueFamily : uniqueQueueFamiles)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.empty() ? nullptr : queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.empty() ? nullptr : deviceExtensions.data();

    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

    if (vkCreateDevice(*physicalDevice, &createInfo, *_allocator, &_device) == VK_SUCCESS)
    {
        std::cout<<"Created logical device"<<std::endl;
    }
    else
    {
        std::cout<<"Failed to create logical device"<<std::endl;
    }
}

Device::~Device()
{
    if (_device)
    {
        std::cout<<"Calling vkDestroyDevice(..)"<<std::endl;
        vkDestroyDevice(_device, *_allocator);
    }
}

}
