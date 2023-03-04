/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/RecordAndSubmitTask.h>
#include <vsg/app/View.h>
#include <vsg/io/Logger.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/vk/State.h>

using namespace vsg;

RecordAndSubmitTask::RecordAndSubmitTask(Device* in_device, uint32_t numBuffers) :
    device(in_device)
{
    _currentFrameIndex = numBuffers; // numBuffers is used to signify unset value
    for (uint32_t i = 0; i < numBuffers; ++i)
    {
        _indices.emplace_back(numBuffers); // numBuffers is used to signify unset value
    }

    _fences.resize(numBuffers);
    for (uint32_t i = 0; i < numBuffers; ++i)
    {
        _fences[i] = vsg::Fence::create(device);
    }

    earlyTransferTask = vsg::TransferTask::create(in_device, numBuffers);
    lateTransferTask = vsg::TransferTask::create(in_device, numBuffers);
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

    if (earlyTransferTask) earlyTransferTask->advance();
    if (lateTransferTask) lateTransferTask->advance();
}

size_t RecordAndSubmitTask::index(size_t relativeFrameIndex) const
{
    return relativeFrameIndex < _indices.size() ? _indices[relativeFrameIndex] : _indices.size();
}

/// fence() and fence(0) return the Fence for the frame currently being rendered, fence(1) return the previous frame's Fence etc.
Fence* RecordAndSubmitTask::fence(size_t relativeFrameIndex)
{
    size_t i = index(relativeFrameIndex);
    return i < _fences.size() ? _fences[i].get() : nullptr;
}

VkResult RecordAndSubmitTask::submit(ref_ptr<FrameStamp> frameStamp)
{
    CommandBuffers recordedCommandBuffers;
    if (VkResult result = start(); result != VK_SUCCESS) return result;

    if (earlyTransferTask)
    {
        if (VkResult result = earlyTransferTask->transferDynamicData(); result != VK_SUCCESS) return result;
    }

    if (VkResult result = record(recordedCommandBuffers, frameStamp); result != VK_SUCCESS) return result;

    return finish(recordedCommandBuffers);
}

VkResult RecordAndSubmitTask::start()
{
    if (earlyTransferTask) earlyTransferTask->currentTransferCompletedSemaphore = {};
    if (lateTransferTask) lateTransferTask->currentTransferCompletedSemaphore = {};

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
    if (lateTransferTask)
    {
        if (VkResult result = lateTransferTask->transferDynamicData(); result != VK_SUCCESS) return result;
    }

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

    auto current_fence = fence();

    // convert VSG CommandBuffer to Vulkan handles and add to the Fence's list of dependent CommandBuffers
    for (auto& commandBuffer : recordedCommandBuffers)
    {
        if (commandBuffer->level() == VK_COMMAND_BUFFER_LEVEL_PRIMARY) vk_commandBuffers.push_back(*commandBuffer);

        current_fence->dependentCommandBuffers().emplace_back(commandBuffer);
    }

    current_fence->dependentSemaphores() = signalSemaphores;

    if (earlyTransferTask && earlyTransferTask->currentTransferCompletedSemaphore)
    {
        vk_waitSemaphores.emplace_back(*earlyTransferTask->currentTransferCompletedSemaphore);
        vk_waitStages.emplace_back(earlyTransferTask->currentTransferCompletedSemaphore->pipelineStageFlags());
    }

    if (lateTransferTask && lateTransferTask->currentTransferCompletedSemaphore)
    {
        vk_waitSemaphores.emplace_back(*lateTransferTask->currentTransferCompletedSemaphore);
        vk_waitStages.emplace_back(lateTransferTask->currentTransferCompletedSemaphore->pipelineStageFlags());
    }

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

    return queue->submit(submitInfo, current_fence);
}

void vsg::updateTasks(RecordAndSubmitTasks& tasks, ref_ptr<CompileManager> compileManager, const CompileResult& compileResult)
{
    //info("vsg::updateTasks(RecordAndSubmitTasks& tasks..) ");
    if (compileResult.earlyDynamicData || compileResult.lateDynamicData)
    {
        for (auto& task : tasks)
        {
            if (task->earlyTransferTask && compileResult.earlyDynamicData)
            {
                task->earlyTransferTask->assign(compileResult.earlyDynamicData);
            }

            if (task->lateTransferTask && compileResult.lateDynamicData)
            {
                task->lateTransferTask->assign(compileResult.lateDynamicData);
            }
        }
    }

    // assign database pager if required
    for (auto& task : tasks)
    {
        for (auto& commandGraph : task->commandGraphs)
        {
            if (compileResult.maxSlot > commandGraph->maxSlot)
            {
                commandGraph->maxSlot = compileResult.maxSlot;
            }
        }
    }

    // assign database pager if required
    if (compileResult.containsPagedLOD)
    {
        vsg::ref_ptr<vsg::DatabasePager> databasePager;
        for (auto& task : tasks)
        {
            if (task->databasePager && !databasePager) databasePager = task->databasePager;
        }

        if (!databasePager)
        {
            databasePager = vsg::DatabasePager::create();
            for (auto& task : tasks)
            {
                if (!task->databasePager)
                {
                    task->databasePager = databasePager;
                    task->databasePager->compileManager = compileManager;
                }
            }

            databasePager->start();
        }
    }

    /// handle any need Bin needs
    for (auto& [const_view, binDetails] : compileResult.views)
    {
        auto view = const_cast<vsg::View*>(const_view);
        for (auto& binNumber : binDetails.indices)
        {
            bool binNumberMatched = false;
            for (auto& bin : view->bins)
            {
                if (bin->binNumber == binNumber)
                {
                    binNumberMatched = true;
                }
            }
            if (!binNumberMatched)
            {
                vsg::Bin::SortOrder sortOrder = (binNumber < 0) ? vsg::Bin::ASCENDING : ((binNumber == 0) ? vsg::Bin::NO_SORT : vsg::Bin::DESCENDING);
                view->bins.push_back(vsg::Bin::create(binNumber, sortOrder));
            }
        }
    }
}
