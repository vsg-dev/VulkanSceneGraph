/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield
Copyright(c) 2020 Julien Valentin

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/RecordTraversal.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/viewer/RecordAndSubmitTask.h>
#include <vsg/viewer/RenderGraph.h>
#include <vsg/vk/State.h>
#include <iostream>

using namespace vsg;

/// sync only primary all secondary already sync with it
class PrimaryRecordedLatch : public Inherit<Object, PrimaryRecordedLatch>
{
public:
    std::mutex CBsProtect;
    CommandBuffers recordedCommandBuffers;

    PrimaryRecordedLatch() {
        _mutex.lock();
    }

    void reset()
    {
        if(_mutex.try_lock())
            _mutex.lock();
        recordedCommandBuffers.clear();
    }

    inline void unleash() { _mutex.unlock(); }
    inline void wait() { _mutex.lock(); }

protected:
    virtual ~PrimaryRecordedLatch() {}
    std::mutex _mutex;
};

struct RecordOperation : public Operation
{
    RecordOperation(CommandGraph* f, ref_ptr<FrameStamp>& fs, ref_ptr<DatabasePager>& obj, ref_ptr<PrimaryRecordedLatch> l) :
        commandGraph(f),
        frameStamp(fs),
        databasePager(obj),
        latch(l) {}

    void run() override
    {
        if(recordedCommandBuffers.empty()){
            commandGraph->record(recordedCommandBuffers, frameStamp, databasePager);
            {
                std::scoped_lock mute(latch->CBsProtect);
                latch->recordedCommandBuffers.insert(std::end(latch->recordedCommandBuffers), std::begin(recordedCommandBuffers), std::end(recordedCommandBuffers));
            }
            //primary is enough as sync with secondary already done
            if(commandGraph->_commandBuffersLevel == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
                latch->unleash();
        }
    }
    ref_ptr<CommandGraph> commandGraph;
    CommandBuffers recordedCommandBuffers;
    ref_ptr<FrameStamp> frameStamp;
    ref_ptr<DatabasePager> databasePager;
    ref_ptr<PrimaryRecordedLatch> latch;
};

RecordAndSubmitTask::RecordAndSubmitTask()
{
    latch = new PrimaryRecordedLatch();
}
void RecordAndSubmitTask::setUpThreading()
{
    recordThreads = new OperationThreads(commandGraphs.size());
}

VkResult RecordAndSubmitTask::submit(ref_ptr<FrameStamp> frameStamp)
{
#if 0
    std::cout << "\n.....................................................\n";
    std::cout << "RecordAndSubmitTask::submit()" << std::endl;
#endif

    std::vector<VkSemaphore> vk_waitSemaphores;
    std::vector<VkPipelineStageFlags> vk_waitStages;
    std::vector<VkSemaphore> vk_signalSemaphores;

    // aquire fence
    ref_ptr<Fence> fence;
    for (auto& window : windows)
    {
        auto& semaphore = window->frame(window->nextImageIndex()).imageAvailableSemaphore;

        vk_waitSemaphores.emplace_back(*semaphore);
        vk_waitStages.emplace_back(semaphore->pipelineStageFlags());

        fence = window->frame(window->nextImageIndex()).commandsCompletedFence;
    }

    // wait on fence and clear semaphores and command buffers
    if (fence)
    {
        if ((fence->dependentSemaphores().size() + fence->dependentCommandBuffers().size()) > 0)
        {
#if 0
            std::cout << "    wait on fence = " << fence.get() << " " << fence->dependentSemaphores().size() << ", " << fence->dependentCommandBuffers().size() << std::endl;
#endif
            uint64_t timeout = 10000000000;
            VkResult result = VK_SUCCESS;
            while ((result = fence->wait(timeout)) == VK_TIMEOUT)
            {
                std::cout << "RecordAndSubmitTask::submit(ref_ptr<FrameStamp> frameStamp)) fence->wait(" << timeout << ") failed with result = " << result << std::endl;
            }
        }
        for (auto& semaphore : fence->dependentSemaphores())
        {
            //std::cout<<"RecordAndSubmitTask::submits(..) "<<*(semaphore->data())<<" "<<semaphore->numDependentSubmissions().load()<<std::endl;
            semaphore->numDependentSubmissions().exchange(0);
        }

        for (auto& commandBuffer : fence->dependentCommandBuffers())
        {
#if 0
            std::cout << "RecordAndSubmitTask::submits(..) " << commandBuffer.get() << " " << std::dec << commandBuffer->numDependentSubmissions().load() << std::endl;
#endif
            commandBuffer->numDependentSubmissions().exchange(0);
        }

        fence->dependentSemaphores().clear();
        fence->dependentCommandBuffers().clear();
        fence->reset();
    }

    for (auto& semaphore : waitSemaphores)
    {
        vk_waitSemaphores.emplace_back(*(semaphore));
        vk_waitStages.emplace_back(semaphore->pipelineStageFlags());
    }

    ref_ptr<PrimaryRecordedLatch> latch_(static_cast<PrimaryRecordedLatch*>(latch.get()));
    latch_->reset();

    // record the commands to the command buffers
    ref_ptr<CommandGraph> lastprimary;
    for (auto& commandGraph : commandGraphs)
    {
        if (! commandGraph->recordTraversal)
        {
             commandGraph->recordTraversal = new RecordTraversal(nullptr, commandGraph->_maxSlot);
        }
        if(commandGraph->_masterCommandGraph && commandGraph->_masterCommandGraph->_commandBuffersLevel == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
        {
            dmat4 projMatrix, viewMatrix;
            static_cast<RenderGraph*>(commandGraph->_masterCommandGraph->getChild(0))->camera->getProjectionMatrix()->get(projMatrix);
            static_cast<RenderGraph*>(commandGraph->_masterCommandGraph->getChild(0))->camera->getViewMatrix()->get(viewMatrix);

            commandGraph->recordTraversal->setProjectionAndViewMatrix(projMatrix, viewMatrix);
            lastprimary = commandGraph->_masterCommandGraph;
        }
        if(lastprimary == commandGraph)
            //force primary not to update
            commandGraph->recordTraversal->state->dirty = false;
        if(recordThreads.valid())
            recordThreads->add(ref_ptr<Operation>(new RecordOperation(commandGraph, frameStamp, databasePager, latch_)));
        else
            commandGraph->record(latch_->recordedCommandBuffers, frameStamp, databasePager);
    }

    if(recordThreads.valid())
    {
        recordThreads->run();
        // wait till all the record operations have completed
        latch_->wait();
    }

    // convert VSG CommandBuffer to Vulkan handles and add to the Fence's list of depdendent CommandBuffers
    std::vector<VkCommandBuffer> vk_commandBuffers;
    for (auto& commandBuffer : latch_->recordedCommandBuffers)
    {
        if(commandBuffer->getCommandBufferLevel() == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
            vk_commandBuffers.push_back(*commandBuffer);

        fence->dependentCommandBuffers().emplace_back(commandBuffer);
    }

    fence->dependentSemaphores() = signalSemaphores;

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
            fence->dependentSemaphores().emplace_back(semaphore);
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
    std::cout << "pdo.graphicsQueue->submit(..) fence = " << fence.get() << "\n";
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
    return queue->submit(submitInfo, fence);
}
