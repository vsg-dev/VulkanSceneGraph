#pragma once

#include <vsg/vk/PhysicalDevice.h>

namespace vsg
{
    class Device : public Object
    {
    public:
        Device(Instance* instance, VkDevice device, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Device, VkResult, VK_SUCCESS>;
        static Result create(Instance* instance, PhysicalDevice* physicalDevice, Names& layers, Names& deviceExtensions, AllocationCallbacks* allocator=nullptr);

        const Instance* getInstance() const { return _instance.get(); }

        operator VkDevice() const { return _device; }
        VkDevice getDevice() const { return _device; }

        AllocationCallbacks* getAllocationCallbacks() { return _allocator.get(); }
        const AllocationCallbacks* getAllocationCallbacks() const { return _allocator.get(); }

        VkQueue getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex=0);

    protected:

        virtual ~Device();

        vsg::ref_ptr<Instance>              _instance;
        VkDevice                            _device;
        vsg::ref_ptr<AllocationCallbacks>   _allocator;
    };

}
