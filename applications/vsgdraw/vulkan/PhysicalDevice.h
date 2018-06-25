#pragma once

#include "vulkan/Surface.h"

/////////////////////////////////////////////////////////////////////
//
// start of vulkan code
//
namespace vsg
{
    class PhysicalDevice : public vsg::Object
    {
    public:
        PhysicalDevice(Instance* instance, Surface* surface, VkPhysicalDevice device, int gFamily, int pFamily) : _instance(instance), _surface(surface), _device(device), _graphicsFamily(gFamily), _presentFamily(pFamily) {}

        PhysicalDevice(Instance* instance, Surface* surface) :
            _instance(instance),
            _surface(surface),
            _device(VK_NULL_HANDLE),
            _graphicsFamily(-1),
            _presentFamily(-1)
        {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);

            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(*instance, &deviceCount, devices.data());

            for (const auto& device : devices)
            {
                _device = device;

                // Checked the DeviceQueueFamilyProperties for support for graphics
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

                std::vector<VkQueueFamilyProperties> queueFamiles(queueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamiles.data());

                for (int i=0; i<queueFamilyCount; ++i)
                {
                    const auto& queueFamily = queueFamiles[i];
                    if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)!=0)
                    {
                        _graphicsFamily = i;
                    }

                    VkBool32 presentSupported = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *surface, &presentSupported);
                    if (queueFamily.queueCount>0 && presentSupported)
                    {
                        _presentFamily = i;
                    }

                    if (complete())
                    {
                        std::cout<<"created PhysicalDevice"<<std::endl;
                        return;
                    }
                }

            }

            _device = VK_NULL_HANDLE;
            _graphicsFamily = -1;
            _presentFamily = -1;

            std::cout<<"creation of PhysicalDevice failed"<<std::endl;

        }

        bool complete() const
        {
            return _device!=VK_NULL_HANDLE && _graphicsFamily>=0 && _presentFamily>=0;
        }

        operator VkPhysicalDevice() const { return _device; }

        VkPhysicalDevice getPhysicalDevice() const { return _device; }
        int getGraphicsFamily() const { return _graphicsFamily; }
        int getPresentFamily() const { return _presentFamily; }

    protected:

        virtual ~PhysicalDevice()
        {
            std::cout<<"PhysicalDevice()::~PhysicalDevice()"<<std::endl;
        }

        vsg::ref_ptr<Instance>  _instance;
        vsg::ref_ptr<Surface>   _surface;

        VkPhysicalDevice _device;
        int _graphicsFamily;
        int _presentFamily;
    };


}
