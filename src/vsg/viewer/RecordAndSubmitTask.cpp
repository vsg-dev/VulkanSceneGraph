/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/RecordTraversal.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/viewer/RecordAndSubmitTask.h>
#include <vsg/vk/State.h>

#include <iostream>

using namespace vsg;

RecordAndSubmitTask::RecordAndSubmitTask(Device* device, uint32_t numBuffers)
{
    _currentFrameIndex = numBuffers; // numBuffers is used to signify unset value
    for (uint32_t i = 0; i < numBuffers; ++i)
    {
        _fences.emplace_back(vsg::Fence::create(device));
        _indices.emplace_back(numBuffers); // numBuffers is used to signify unset value
    }
}

void RecordAndSubmitTask::advance()
{
    if (_currentFrameIndex >= _indices.size())
    {
        // first frame so set to 0
        _currentFrameIndex = 0;
    }
    else
    {
        ++_currentFrameIndex;
        if (_currentFrameIndex > _indices.size() - 1) _currentFrameIndex = 0;

        // shift the index for previous frames
        for (size_t i = _indices.size() - 1; i >= 1; --i)
        {
            _indices[i] = _indices[i - 1];
        }
    }

    // pass the index for the current frame
    _indices[0] = _currentFrameIndex;
}

VkResult RecordAndSubmitTask::submit(ref_ptr<FrameStamp> frameStamp)
{
    CommandBuffers recordedCommandBuffers;
    if (VkResult result = start(); result != VK_SUCCESS) return result;
    if (VkResult result = record(recordedCommandBuffers, frameStamp); result != VK_SUCCESS) return result;
    return finish(recordedCommandBuffers);
}

VkResult RecordAndSubmitTask::start()
{
    auto current_fence = fence();
    if (current_fence->hasDependencies())
    {
        uint64_t timeout = std::numeric_limits<uint64_t>::max();
        if (VkResult result = current_fence->wait(timeout); result != VK_SUCCESS) return result;

        current_fence->resetFenceAndDependencies();
    }
    return VK_SUCCESS;
}

VkResult RecordAndSubmitTask::record(CommandBuffers& recordedCommandBuffers, ref_ptr<FrameStamp> frameStamp)
{
    for (auto& commandGraph : commandGraphs)
    {
        commandGraph->record(recordedCommandBuffers, frameStamp, databasePager);
    }
    return VK_SUCCESS;
}

VkResult RecordAndSubmitTask::finish(CommandBuffers& recordedCommandBuffers)
{
    auto current_fence = fence();

    if (recordedCommandBuffers.empty())
    {
        // nothing to do so return early
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // sleep for 1/60th of a second
        return VK_SUCCESS;
    }

    // convert VSG CommandBuffer to Vulkan handles and add to the Fence's list of dependent CommandBuffers
    std::vector<VkCommandBuffer> vk_commandBuffers;
    std::vector<VkSemaphore> vk_waitSemaphores;
    std::vector<VkPipelineStageFlags> vk_waitStages;
    std::vector<VkSemaphore> vk_signalSemaphores;

    // convert VSG CommandBuffer to Vulkan handles and add to the Fence's list of dependent CommandBuffers
    for (auto& commandBuffer : recordedCommandBuffers)
    {
        if (commandBuffer->level() == VK_COMMAND_BUFFER_LEVEL_PRIMARY) vk_commandBuffers.push_back(*commandBuffer);

        current_fence->dependentCommandBuffers().emplace_back(commandBuffer);
    }

    current_fence->dependentSemaphores() = signalSemaphores;

    for (auto& window : windows)
    {
        auto imageIndex = window->imageIndex();
        if (imageIndex >= window->numFrames()) continue;

        auto& semaphore = window->frame(imageIndex).imageAvailableSemaphore;

        vk_waitSemaphores.emplace_back(*semaphore);
        vk_waitStages.emplace_back(semaphore->pipelineStageFlags());
    }

    for (auto& semaphore : waitSemaphores)
    {
        vk_waitSemaphores.emplace_back(*(semaphore));
        vk_waitStages.emplace_back(semaphore->pipelineStageFlags());
    }

    if (databasePager)
    {
        for (auto& semaphore : databasePager->getSemaphores())
        {
            if (semaphore->numDependentSubmissions().load() > 1)
            {
                std::cout << "    Warning: Viewer::submitNextFrame() waitSemaphore " << *(semaphore->data()) << " " << semaphore->numDependentSubmissions().load() << std::endl;
            }
            else
            {
                // std::cout<<"    Viewer::submitNextFrame() waitSemaphore "<<*(semaphore->data())<<" "<<semaphore->numDependentSubmissions().load()<<std::endl;
            }

            vk_waitSemaphores.emplace_back(*semaphore);
            vk_waitStages.emplace_back(semaphore->pipelineStageFlags());

            semaphore->numDependentSubmissions().fetch_add(1);
            current_fence->dependentSemaphores().emplace_back(semaphore);
        }
    }

    for (auto& semaphore : signalSemaphores)
    {
        vk_signalSemaphores.emplace_back(*(semaphore));
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(vk_waitSemaphores.size());
    submitInfo.pWaitSemaphores = vk_waitSemaphores.data();
    submitInfo.pWaitDstStageMask = vk_waitStages.data();

    submitInfo.commandBufferCount = static_cast<uint32_t>(vk_commandBuffers.size());
    submitInfo.pCommandBuffers = vk_commandBuffers.data();

    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(vk_signalSemaphores.size());
    submitInfo.pSignalSemaphores = vk_signalSemaphores.data();

#if 0
    std::cout << "pdo.graphicsQueue->submit(..) current_fence = " << current_fence << "\n";
    std::cout << "    submitInfo.waitSemaphoreCount = " << submitInfo.waitSemaphoreCount << "\n";
    for (uint32_t i = 0; i < submitInfo.waitSemaphoreCount; ++i)
    {
        std::cout << "        submitInfo.pWaitSemaphores[" << i << "] = " << submitInfo.pWaitSemaphores[i] << "\n";
        std::cout << "        submitInfo.pWaitDstStageMask[" << i << "] = " << submitInfo.pWaitDstStageMask[i] << "\n";
    }
    std::cout << "    submitInfo.commandBufferCount = " << submitInfo.commandBufferCount << "\n";
    for (uint32_t i = 0; i < submitInfo.commandBufferCount; ++i)
    {
        std::cout << "        submitInfo.pCommandBuffers[" << i << "] = " << submitInfo.pCommandBuffers[i] << "\n";
    }
    std::cout << "    submitInfo.signalSemaphoreCount = " << submitInfo.signalSemaphoreCount << "\n";
    for (uint32_t i = 0; i < submitInfo.signalSemaphoreCount; ++i)
    {
        std::cout << "        submitInfo.pSignalSemaphores[" << i << "] = " << submitInfo.pSignalSemaphores[i] << "\n";
    }
    std::cout << std::endl;
#endif

    return queue->submit(submitInfo, current_fence);
}
