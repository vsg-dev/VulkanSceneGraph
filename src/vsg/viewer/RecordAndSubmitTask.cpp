/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/viewer/RecordAndSubmitTask.h>

#include <vsg/traversals/DispatchTraversal.h>

#include <vsg/vk/State.h>

#include <vsg/ui/ApplicationEvent.h>

using namespace vsg;

#include <iostream>

VkResult RecordAndSubmitTask::submit(ref_ptr<FrameStamp> frameStamp)
{
    std::cout<<"\n.....................................................\n";
    std::cout<<"RecordAndSubmitTask::submit()"<<std::endl;

    std::vector<VkSemaphore> vk_waitSemaphores;
    std::vector<VkPipelineStageFlags> vk_waitStages;
    std::vector<VkCommandBuffer> vk_commandBuffers;
    std::vector<VkSemaphore> vk_signalSemaphores;

    static int s_first_frame = 0;

    // aquire fence
    ref_ptr<Fence> fence;
    for (auto& window : windows)
    {
        vk_waitSemaphores.push_back(*(window->frame(window->nextImageIndex()).imageAvailableSemaphore));
        vk_waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        fence = window->frame(window->nextImageIndex()).commandsCompletedFence;
    }

    // wait on fence and clear semaphores and command buffers
    if (fence)
    {
        if ((fence->dependentSemaphores().size() + fence->dependentCommandBuffers().size()) > 0)
        {
            std::cout<<"    wait on fence = "<<fence.get()<<" "<<fence->dependentSemaphores().size()<<", "<<fence->dependentCommandBuffers().size()<<std::endl;
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
            std::cout<<"RecordAndSubmitTask::submits(..) "<<commandBuffer.get()<<" "<<std::dec<<commandBuffer->numDependentSubmissions().load()<<std::endl;
            commandBuffer->numDependentSubmissions().exchange(0);
        }

        fence->dependentSemaphores().clear();
        fence->dependentCommandBuffers().clear();
        fence->reset();
    }

    for(auto& semaphore : waitSemaphores)
    {
        vk_waitSemaphores.emplace_back(*(semaphore));
        vk_waitStages.emplace_back(semaphore->pipelineStageFlags());
    }

    // record the commands in the command buffer
    for(auto& commandGraph : commandGraphs)
    {
        // pass the inext to dispatchTraversal visitor?  Only for RenderGraph?
        DispatchTraversal dispatchTraversal(nullptr, commandGraph->_maxSlot, frameStamp);

        dispatchTraversal.databasePager = databasePager;
        if (databasePager) dispatchTraversal.culledPagedLODs = databasePager->culledPagedLODs;

        commandGraph->accept(dispatchTraversal);

        vk_commandBuffers.push_back(*(dispatchTraversal.state->_commandBuffer));

        fence->dependentCommandBuffers().emplace_back(dispatchTraversal.state->_commandBuffer);
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


    for(auto& semaphore : signalSemaphores)
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

    std::cout<<"pdo.graphicsQueue->submit(..) fence = "<<fence.get()<<"\n";
    std::cout<<"    submitInfo.waitSemaphoreCount = "<<submitInfo.waitSemaphoreCount<<"\n";
    for(uint32_t i=0; i<submitInfo.waitSemaphoreCount; ++i)
    {
        std::cout<<"        submitInfo.pWaitSemaphores["<<i<<"] = "<<submitInfo.pWaitSemaphores[i]<<"\n";
        std::cout<<"        submitInfo.pWaitDstStageMask["<<i<<"] = "<<submitInfo.pWaitDstStageMask[i]<<"\n";
    }
    std::cout<<"    submitInfo.commandBufferCount = "<<submitInfo.commandBufferCount<<"\n";
    for(uint32_t i=0; i<submitInfo.commandBufferCount; ++i)
    {
        std::cout<<"        submitInfo.pCommandBuffers["<<i<<"] = "<<submitInfo.pCommandBuffers[i]<<"\n";
    }
    std::cout<<"    submitInfo.signalSemaphoreCount = "<<submitInfo.signalSemaphoreCount<<"\n";
    for(uint32_t i=0; i<submitInfo.signalSemaphoreCount; ++i)
    {
        std::cout<<"        submitInfo.pSignalSemaphores["<<i<<"] = "<<submitInfo.pSignalSemaphores[i]<<"\n";
    }
    std::cout<<std::endl;

    return queue->submit(submitInfo, fence);
}

