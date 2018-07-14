#pragma once

#include <vsg/vk/Device.h>
#include <vsg/vk/CmdDraw.h>

namespace vsg
{
    class Buffer : public Dispatch
    {
    public:
        Buffer(Device* device, VkBuffer Buffer, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Buffer, VkResult, VK_SUCCESS>;
        static Result create(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, AllocationCallbacks* allocator=nullptr);

        static Result createVertexBuffer(Device* device, VkDeviceSize size)
        {
            return Buffer::create(device, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
        }


        operator VkBuffer () const { return _buffer; }

        virtual void dispatch(VkCommandBuffer commandBuffer) const
        {
            VkBuffer vertexBuffers[] = {_buffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        }

    protected:
        virtual ~Buffer();

        ref_ptr<Device>                 _device;
        VkBuffer                        _buffer;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
