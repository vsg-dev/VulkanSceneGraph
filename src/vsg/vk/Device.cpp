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

Device::~Device()
{
    if (_device)
    {
        std::cout<<"Calling vkDestroyDevice(..)"<<std::endl;
        vkDestroyDevice(_device, *_allocator);
    }
}

Device::Result Device::create(Instance* instance, PhysicalDevice* physicalDevice, Names& layers, Names& deviceExtensions, AllocationCallbacks* allocator)
{
    if (!instance || !physicalDevice)
    {
        return Device::Result("Error: vsg::Device::create(...) failed to create logical device, undefined Instance and/or PhysicalDevice.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    std::set<int> uniqueQueueFamiles;
    if (physicalDevice->getGraphicsFamily()>=0) uniqueQueueFamiles.insert(physicalDevice->getGraphicsFamily());
    if (physicalDevice->getComputeFamily()>=0) uniqueQueueFamiles.insert(physicalDevice->getComputeFamily());
    if (physicalDevice->getPresentFamily()>=0) uniqueQueueFamiles.insert(physicalDevice->getPresentFamily());

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
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.empty() ? nullptr : queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.empty() ? nullptr : deviceExtensions.data();

    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

    VkDevice device;
    VkResult result = vkCreateDevice(*physicalDevice, &createInfo, *allocator, &device);
    if (result == VK_SUCCESS)
    {
        std::cout<<"Created logical device"<<std::endl;
        return new Device(instance, device, allocator);
    }
    else
    {
        return Device::Result("Error: vsg::Device::create(...) failed to create logical device.", result);
    }
}

VkQueue Device::getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex)
{
    VkQueue queue;
    vkGetDeviceQueue(_device, queueFamilyIndex, queueIndex, &queue);
    return queue;
}




}
