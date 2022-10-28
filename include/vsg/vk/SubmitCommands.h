#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Object.h>
#include <vsg/vk/Fence.h>
#include <vsg/vk/Queue.h>

#include <mutex>
#include <vector>

namespace vsg
{
    /// convenience template function for submitting Vulkan commands to a queue
    template<typename F>
    void submitCommandsToQueue(CommandPool* commandPool, Fence* fence, uint64_t timeout, Queue* queue, F function)
    {
        auto commandBuffer = commandPool->allocate();

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

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
            queue->submit(submitInfo, fence);
            if (timeout > 0) fence->wait(timeout);
        }
        else
        {
            queue->submit(submitInfo, VK_NULL_HANDLE);
            queue->waitIdle();
        }
    }

    /// convenience template function for submitting Vulkan commands to a queue and wait for completion.
    template<typename F>
    void submitCommandsToQueue(CommandPool* commandPool, Queue* queue, F function)
    {
        submitCommandsToQueue(commandPool, nullptr, 0, queue, function);
    }

} // namespace vsg
