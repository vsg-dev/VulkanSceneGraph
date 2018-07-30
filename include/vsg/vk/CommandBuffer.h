#pragma once

#include <vsg/vk/CommandPool.h>

namespace vsg
{

    class CommandBuffer : public Group
    {
    public:
        CommandBuffer(Device* device, CommandPool* commandPool, VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags flags);

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        using Result = vsg::Result<CommandBuffer, VkResult, VK_SUCCESS>;
        static Result create(Device* device,  CommandPool* commandPool, VkCommandBufferUsageFlags flags);

        VkCommandBufferUsageFlags flags() const { return _flags; }

        std::size_t size() const { return 1; }
        const VkCommandBuffer* data() const { return &_commandBuffer; }

        operator VkCommandBuffer () const { return _commandBuffer; }

    protected:
        virtual ~CommandBuffer();

        ref_ptr<Device>             _device;
        ref_ptr<CommandPool>        _commandPool;
        VkCommandBuffer             _commandBuffer;
        VkCommandBufferUsageFlags   _flags;
    };



    class CommandBuffers : public Object
    {
    public:
        using Buffers = std::vector<VkCommandBuffer>;

        CommandBuffers(Device* device, CommandPool* commandPool, const Buffers& buffers);

        using Result = vsg::Result<CommandBuffers, VkResult, VK_SUCCESS>;
        static Result create(Device* device,  CommandPool* commandPool, size_t size);

        std::size_t size() const { return _buffers.size(); }
        const VkCommandBuffer* data() const { return _buffers.data(); }

        const VkCommandBuffer& operator [] (size_t i) const { return _buffers[i]; }
        const VkCommandBuffer& at(size_t i) const { return _buffers[i]; }

    protected:
        virtual ~CommandBuffers();

        ref_ptr<Device>         _device;
        ref_ptr<CommandPool>    _commandPool;
        Buffers                 _buffers;
    };



}
