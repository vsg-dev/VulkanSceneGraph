#include <vsg/vk/PhysicalDevice.h>

#include <iostream>

namespace vsg
{

PhysicalDevice::PhysicalDevice(Instance* instance, Surface* surface, VkPhysicalDevice device, int gFamily, int pFamily) :
    _instance(instance),
    _surface(surface),
    _device(device),
    _graphicsFamily(gFamily),
    _presentFamily(pFamily)
{
    vkGetPhysicalDeviceProperties(_device, &_properties);
}

PhysicalDevice::Result PhysicalDevice::create(Instance* instance, Surface* surface)
{
    if (!instance || !surface)
    {
        return PhysicalDevice::Result("Error: vsg::PhysicalDevice::create(...) failed to create physical device, undefined Instance and/or Surface.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(*instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        // Checked the DeviceQueueFamilyProperties for support for graphics
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamiles(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamiles.data());

        int graphicsFamily = -1;
        int presentFamily = -1;

        for (int i=0; i<queueFamilyCount; ++i)
        {
            const auto& queueFamily = queueFamiles[i];
            if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)!=0)
            {
                graphicsFamily = i;
            }

            VkBool32 presentSupported = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *surface, &presentSupported);
            if (queueFamily.queueCount>0 && presentSupported)
            {
                presentFamily = i;
            }

            if (device!=VK_NULL_HANDLE && graphicsFamily>=0 && presentFamily>=0)
            {
                std::cout<<"created PhysicalDevice"<<std::endl;
                return new PhysicalDevice(instance, surface, device, graphicsFamily, presentFamily);
            }
        }

    }

    return PhysicalDevice::Result("Error: vsg::Device::create(...) failed to create physical device.", VK_INCOMPLETE);
}

PhysicalDevice::~PhysicalDevice()
{
    std::cout<<"PhysicalDevice()::~PhysicalDevice()"<<std::endl;
}

}
