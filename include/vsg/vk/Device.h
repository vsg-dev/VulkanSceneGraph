#pragma once

#include <vsg/vk/PhysicalDevice.h>

namespace vsg
{

    class Device : public Object
    {
    public:
        Device(Instance* instance, VkDevice device, VkAllocationCallbacks* pAllocator=nullptr);

        Device(Instance* instance, PhysicalDevice* physicalDevice, Names& layers, Names& deviceExtensions, VkAllocationCallbacks* pAllocator=nullptr);

        operator VkDevice() const { return _device; }

    protected:

        virtual ~Device();

        vsg::ref_ptr<Instance>          _instance;
        vsg::ref_ptr<PhysicalDevice>    _physicalDevice;
        VkDevice                        _device;
        VkAllocationCallbacks*          _pAllocator;
    };

}
