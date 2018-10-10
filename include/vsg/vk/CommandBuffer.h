#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/CommandPool.h>
#include <vsg/vk/Fence.h>
#include <vsg/vk/Pipeline.h>

namespace vsg
{

    class VSG_EXPORT CommandBuffer : public Inherit<Object, CommandBuffer>
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

        void setCurrentPipeline(const Pipeline* pipeline)
        {
            _currentPipeline = pipeline;
            _currentPipelineLayout = (pipeline!=nullptr) ? pipeline->getPipelineLayout() : nullptr;
        }

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

    class VSG_EXPORT CommandBuffers : public Inherit<Object, CommandBuffers>
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
