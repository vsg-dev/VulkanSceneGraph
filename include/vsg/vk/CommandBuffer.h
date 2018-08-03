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

    template<typename F>
    void dispatchCommandsToQueue(Device* device, CommandPool* commandPool, VkQueue queue, F function)
    {
        vsg::ref_ptr<vsg::CommandBuffer> transferCommand = vsg::CommandBuffer::create(device, commandPool, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = transferCommand->flags();

        vkBeginCommandBuffer(*transferCommand, &beginInfo);

            function(*transferCommand);

        vkEndCommandBuffer(*transferCommand);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = transferCommand->data();

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

        // we must wait for the queue to empty before we can safely clean up the transferCommand
        vkQueueWaitIdle(queue);
    }




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
