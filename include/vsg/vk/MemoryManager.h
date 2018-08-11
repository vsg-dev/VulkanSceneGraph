#pragma once

#include <vsg/core/Data.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/DeviceMemory.h>

namespace vsg
{
    class MemoryManager : public Object
    {
    public:
        MemoryManager(PhysicalDevice* physicalDevice, Device* device, AllocationCallbacks* allocator=nullptr);

        Buffer* createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode);

        DeviceMemory* createMemory(const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties);

    protected:
        virtual ~MemoryManager();

        ref_ptr<PhysicalDevice>         _physcicalDevice;
        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
