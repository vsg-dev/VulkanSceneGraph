#pragma once

#include <vsg/vk/CommandPool.h>
#include <vsg/vk/Fence.h>
#include <vsg/vk/Pipeline.h>

namespace vsg
{

    class CommandBuffer : public Object
    {
    public:
        CommandBuffer(Device* device, CommandPool* commandPool, VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags flags);

        using Result = vsg::Result<CommandBuffer, VkResult, VK_SUCCESS>;
        static Result create(Device* device,  CommandPool* commandPool, VkCommandBufferUsageFlags flags);

        VkCommandBufferUsageFlags flags() const { return _flags; }

        const VkCommandBuffer* data() const { return &_commandBuffer; }

        operator VkCommandBuffer () const { return _commandBuffer; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

        void setCurrentPipeline(const Pipeline* pipeline) { _currentPipeline = pipeline; }
        const Pipeline* getCurrentPipeline() const { return _currentPipeline; }
        const PipelineLayout* getCurrentPipelineLayout() const { return _currentPipelineLayout; }

    protected:
        virtual ~CommandBuffer();

        VkCommandBuffer                 _commandBuffer;
        VkCommandBufferUsageFlags       _flags;

        ref_ptr<Device>                 _device;
        ref_ptr<CommandPool>            _commandPool;
        ref_ptr<const Pipeline>         _currentPipeline;
        ref_ptr<const PipelineLayout>   _currentPipelineLayout;
    };

    template<typename F>
    void dispatchCommandsToQueue(Device* device, CommandPool* commandPool, Fence* fence, uint64_t timeout, VkQueue queue, F function)
    {
        vsg::ref_ptr<vsg::CommandBuffer> commandBuffer = vsg::CommandBuffer::create(device, commandPool, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = commandBuffer->flags();

        vkBeginCommandBuffer(*commandBuffer, &beginInfo);

            function(*commandBuffer);

        vkEndCommandBuffer(*commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = commandBuffer->data();

        // we must wait for the queue to empty before we can safely clean up the commandBuffer
        if (fence)
        {
            vkQueueSubmit(queue, 1, &submitInfo, *fence);
            fence->wait(timeout);
        }
        else
        {
            vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(queue);
        }
    }


    template<typename F>
    void dispatchCommandsToQueue(Device* device, CommandPool* commandPool, VkQueue queue, F function)
    {
        dispatchCommandsToQueue(device, commandPool, nullptr, 0, queue, function);
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
