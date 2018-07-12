#pragma once

#include <vsg/vk/Device.h>
#include <vsg/vk/Swapchain.h>
#include <vsg/vk/RenderPass.h>

namespace vsg
{
    class Buffer : public Object
    {
    public:
        Buffer(Device* device, VkBuffer Buffer, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Buffer, VkResult, VK_SUCCESS>;
        static Result create(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, AllocationCallbacks* allocator=nullptr);

        operator VkBuffer () const { return _buffer; }

    protected:
        virtual ~Buffer();

        ref_ptr<Device>                 _device;
        VkBuffer                        _buffer;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
