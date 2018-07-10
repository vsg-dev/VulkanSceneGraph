#pragma once

#include <vsg/vk/PhysicalDevice.h>

namespace vsg
{

    class Device : public Object
    {
    public:
        Device(Instance* instance, VkDevice device, AllocationCallbacks* allocator=nullptr);

        Device(Instance* instance, PhysicalDevice* physicalDevice, Names& layers, Names& deviceExtensions, AllocationCallbacks* allocator=nullptr);

        const Instance* getInstance() const { return _instance.get(); }
        const PhysicalDevice* getPhysicalDevice() const { return _physicalDevice.get(); }

        operator VkDevice() const { return _device; }
        VkDevice getDevice() const { return _device; }

        AllocationCallbacks* getAllocationCallbacks() { return _allocator.get(); }
        const AllocationCallbacks* getAllocationCallbacks() const { return _allocator.get(); }

    protected:

        virtual ~Device();

        vsg::ref_ptr<Instance>              _instance;
        vsg::ref_ptr<PhysicalDevice>        _physicalDevice;
        VkDevice                            _device;
        vsg::ref_ptr<AllocationCallbacks>   _allocator;
    };

}
