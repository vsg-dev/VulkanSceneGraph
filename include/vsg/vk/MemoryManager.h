#pragma once

#include <vsg/core/Data.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/DeviceMemory.h>

namespace vsg
{
    class VSG_EXPORT MemoryManager : public Object
    {
    public:
        MemoryManager(Device* device, AllocationCallbacks* allocator=nullptr);

        Buffer* createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode);

        DeviceMemory* createMemory(const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties);

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~MemoryManager();

        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
