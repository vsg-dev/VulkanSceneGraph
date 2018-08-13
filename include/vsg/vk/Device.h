#pragma once

#include <vsg/vk/PhysicalDevice.h>

namespace vsg
{
    class Device : public Object
    {
    public:
        Device(VkDevice device, PhysicalDevice* physicalDevice, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Device, VkResult, VK_SUCCESS>;
        static Result create(PhysicalDevice* physicalDevice, Names& layers, Names& deviceExtensions, AllocationCallbacks* allocator=nullptr);

        const PhysicalDevice* getPhysicalDevice() const { return _physicalDevice.get(); }

        operator VkDevice() const { return _device; }
        VkDevice getDevice() const { return _device; }

        AllocationCallbacks* getAllocationCallbacks() { return _allocator.get(); }
        const AllocationCallbacks* getAllocationCallbacks() const { return _allocator.get(); }

        VkQueue getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex=0);

    protected:

        virtual ~Device();

        VkDevice                            _device;
        vsg::ref_ptr<PhysicalDevice>        _physicalDevice;
        vsg::ref_ptr<AllocationCallbacks>   _allocator;
    };

}
