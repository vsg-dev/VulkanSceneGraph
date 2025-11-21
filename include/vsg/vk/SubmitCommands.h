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
#include <vsg/vk/State.h>

#include <mutex>
#include <vector>

namespace vsg
{
    /// deprecated: use CommandGraph/Viewer instead
    template<typename F>
    VkResult submitCommandsToQueue(CommandPool* commandPool, Fence* fence, uint64_t timeout, Queue* queue, F function)
    {
        VkResult result = VK_SUCCESS;

        auto commandBuffer = commandPool->allocate();
        auto state = State::create(Slots{4,4});
        state->connect(commandBuffer);

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
            result = queue->submit(submitInfo, fence);
            if (result == VK_SUCCESS && timeout > 0)
            {
                result = fence->wait(timeout);
            }
        }
        else
        {
            result = queue->submit(submitInfo, VK_NULL_HANDLE);
            if (result == VK_SUCCESS)
            {
                result = queue->waitIdle();
            }
        }
        return result;
    }

    /// deprecated: use CommandGraph/Viewer instead
    template<typename F>
    VkResult submitCommandsToQueue(CommandPool* commandPool, Queue* queue, F function)
    {
        return submitCommandsToQueue(commandPool, nullptr, 0, queue, function);
    }

} // namespace vsg
