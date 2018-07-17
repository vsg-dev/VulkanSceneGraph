#pragma once

#include <vsg/vk/Device.h>
#include <vsg/vk/CmdDraw.h>

namespace vsg
{
    class Buffer : public Object
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

    protected:
        virtual ~Buffer();

        ref_ptr<Device>                 _device;
        VkBuffer                        _buffer;
        ref_ptr<AllocationCallbacks>    _allocator;
    };


    class CmdBindVertexBuffers : public Dispatch
    {
    public:

        CmdBindVertexBuffers() : _firstBinding(0) {}

        void setFirstBinding(uint32_t firstBinding) { _firstBinding = firstBinding; }
        uint32_t getFirstBinding() const { return _firstBinding; }

        void add(Buffer* buffer, VkDeviceSize offset)
        {
            _buffers.push_back(buffer);
            _vkBuffers.push_back(*buffer);
            _offsets.push_back(offset);
        }

        virtual void dispatch(VkCommandBuffer commandBuffer) const
        {
            vkCmdBindVertexBuffers(commandBuffer, _firstBinding, _buffers.size(), _vkBuffers.data(), _offsets.data());
        }

    protected:
        virtual ~CmdBindVertexBuffers() {}

        using Buffers = std::vector<ref_ptr<Buffer>>;
        using VkBuffers = std::vector<VkBuffer>;
        using Offsets = std::vector<VkDeviceSize>;

        uint32_t _firstBinding;
        Buffers _buffers;
        VkBuffers _vkBuffers;
        Offsets _offsets;

    };


    class CmdBindIndexBuffer : public Dispatch
    {
    public:

        CmdBindIndexBuffer(Buffer* buffer, VkDeviceSize offset, VkIndexType indexType) : _buffer(buffer), _offset(offset), _indexType(indexType) {}

        virtual void dispatch(VkCommandBuffer commandBuffer) const
        {
            vkCmdBindIndexBuffer(commandBuffer, *_buffer, _offset, _indexType);
        }

    protected:
        virtual ~CmdBindIndexBuffer() {}

        vsg::ref_ptr<Buffer> _buffer;
        VkDeviceSize _offset;
        VkIndexType _indexType;

    };
}
