/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>

#include <vsg/nodes/StateGroup.h>

#include <vsg/vk/Descriptor.h>

#include <vsg/viewer/GraphicsStage.h>
#include <vsg/raytracing/RayTracingStage.h>
#include <vsg/viewer/Viewer.h>

#include <chrono>
#include <iostream>
#include <map>
#include <set>

using namespace vsg;

Viewer::Viewer()
{
    _start_point = clock::now();
}

Viewer::~Viewer()
{
    // don't kill window while devices are still active
    for (auto& pair_pdo : _deviceMap)
    {
        vkDeviceWaitIdle(*pair_pdo.first);
    }
}

void Viewer::addWindow(ref_ptr<Window> window)
{
    _windows.push_back(window);

    ref_ptr<Device> device(window->device());
    PhysicalDevice* physicalDevice = window->physicalDevice();
    if (_deviceMap.find(device) == _deviceMap.end())
    {
        // set up per device settings
        PerDeviceObjects& new_pdo = _deviceMap[device];
        new_pdo.renderFinishedSemaphore = vsg::Semaphore::create(device);
        new_pdo.graphicsQueue = device->getQueue(physicalDevice->getGraphicsFamily());
        new_pdo.presentQueue = device->getQueue(physicalDevice->getPresentFamily());
        new_pdo.signalSemaphores.push_back(*new_pdo.renderFinishedSemaphore);
    }

    // add per window details to pdo
    PerDeviceObjects& pdo = _deviceMap[device];
    pdo.windows.push_back(window);
    pdo.imageIndices.push_back(0);   // to be filled in by submitFrame()
    pdo.commandBuffers.push_back(0); // to be filled in by submitFrame()
    pdo.swapchains.push_back(*(window->swapchain()));
    pdo.waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}

bool Viewer::active() const
{
    bool viewerIsActive = !_close;
    if (viewerIsActive)
    {
        for (auto window : _windows)
        {
            if (!window->valid()) viewerIsActive = false;
        }
    }

    if (!viewerIsActive)
    {
        // don't exit mainloop while the any devices are still active
        for (auto& pair_pdo : _deviceMap)
        {
            vkDeviceWaitIdle(*pair_pdo.first);
        }
        return false;
    }
    else
    {
        return true;
    }
}

bool Viewer::pollEvents(bool discardPreviousEvents)
{
    bool result = false;

    if (discardPreviousEvents) _events.clear();
    for (auto& window : _windows)
    {
        if (window->pollEvents(_events)) result = true;
    }

    return result;
}

void Viewer::reassignFrameCache()
{
    for (auto& pair_pdo : _deviceMap)
    {
        PerDeviceObjects& pdo = pair_pdo.second;
        pdo.imageIndices.clear();
        pdo.commandBuffers.clear();
        pdo.swapchains.clear();
        pdo.waitStages.clear();

        for (auto window : pdo.windows)
        {
            pdo.imageIndices.push_back(0);   // to be filled in by submitFrame()
            pdo.commandBuffers.push_back(0); // to be filled in by submitFrame()
            pdo.swapchains.push_back(*(window->swapchain()));
            pdo.waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        }
    }
}

void Viewer::advance()
{
    // poll all the windows for events.
    pollEvents(true);

    // create FrameStamp for frame
    auto time = vsg::clock::now();
    _frameStamp = _frameStamp ? new vsg::FrameStamp(time, _frameStamp->frameCount + 1) : new vsg::FrameStamp(time, 0);

    // create an event for the new frame.
    _events.emplace_back(new FrameEvent(_frameStamp));
}

bool Viewer::advanceToNextFrame()
{
    if (!active()) return false;

    // poll all the windows for events.
    pollEvents(true);

    if (!acquireNextFrame()) return false;

    // create FrameStamp for frame
    auto time = vsg::clock::now();
    _frameStamp = _frameStamp ? new vsg::FrameStamp(time, _frameStamp->frameCount + 1) : new vsg::FrameStamp(time, 0);

    // create an event for the new frame.
    _events.emplace_back(new FrameEvent(_frameStamp));

    return true;
}

bool Viewer::acquireNextFrame()
{
    if (_close) return false;

    bool needToReassingFrameCache = false;
    VkResult result = VK_SUCCESS;
    for (auto& window : _windows)
    {
        unsigned int numTries = 0;
        unsigned int maximumTries = 10;
        while (((result = window->acquireNextImage()) == VK_ERROR_OUT_OF_DATE_KHR) && (numTries < maximumTries))
        {
            ++numTries;

            // wait till queue are empty before we resize.
            for (auto& pair_pdo : _deviceMap)
            {
                PerDeviceObjects& pdo = pair_pdo.second;
                pdo.presentQueue->waitIdle();
            }

            //std::cout<<"window->acquireNextImage(), result==VK_ERROR_OUT_OF_DATE_KHR  rebuild swap chain : resized="<<window->resized()<<" numTries="<<numTries<<std::endl;

            // resize to rebuild all the internal Vulkan objects associated with the window.
            window->resize();

            needToReassingFrameCache = true;
        }

        if (result != VK_SUCCESS) break;
    }

    if (needToReassingFrameCache)
    {
        // reassign frame cache
        reassignFrameCache();
    }

    return result == VK_SUCCESS;
}

void Viewer::handleEvents()
{
    for (auto& vsg_event : _events)
    {
        for (auto& handler : _eventHandlers)
        {
            vsg_event->accept(*handler);
        }
    }
}

bool Viewer::populateNextFrame()
{
    for (auto& window : _windows)
    {
        window->populateCommandBuffers(window->nextImageIndex(), _frameStamp);
    }
    return true;
}

bool Viewer::submitNextFrame()
{
    bool debugLayersEnabled = false;

    for (auto& pair_pdo : _deviceMap)
    {
        PerDeviceObjects& pdo = pair_pdo.second;

        ref_ptr<Fence> fence;

        std::vector<VkSemaphore> waitSemaphores;
        std::vector<VkPipelineStageFlags> waitDstStageMasks;
        for (auto& window : pdo.windows)
        {
            waitSemaphores.push_back(*(window->frame(window->nextImageIndex()).imageAvailableSemaphore));
            waitDstStageMasks.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

            fence = window->frame(window->nextImageIndex()).commandsCompletedFence;
            window->frame(window->nextImageIndex()).checkCommandsCompletedFence = true;

            // copy semaphore's assigned to database pagers
            for (auto& stage : window->stages())
            {
                GraphicsStage* gs = dynamic_cast<GraphicsStage*>(stage.get());
                DatabasePager* db = gs ? gs->databasePager.get() : nullptr;
                if (db)
                {
                    //std::cout<<"Viewer::submitNextFrame()"<<std::endl;
                    for (auto& semaphore : db->getSemaphores())
                    {
                        if (semaphore->numDependentSubmissions().load() > 1)
                        {
                            std::cout << "    Warning: Viewer::submitNextFrame() waitSemaphore " << *(semaphore->data()) << " " << semaphore->numDependentSubmissions().load() << std::endl;
                        }
                        else
                        {
                            // std::cout<<"    Viewer::submitNextFrame() waitSemaphore "<<*(semaphore->data())<<" "<<semaphore->numDependentSubmissions().load()<<std::endl;
                        }

                        waitSemaphores.emplace_back(*semaphore);
                        waitDstStageMasks.emplace_back(semaphore->pipelineStageFlags());

                        semaphore->numDependentSubmissions().fetch_add(1);
                        fence->dependentSemaphores().emplace_back(semaphore);
                    }
                }
            }
        }

        // if (!hasDBSemaphore) std::cout<<"Viewer::submitNextFrame() no DB wait semaphores required."<<std::endl;

        // fill in the imageIndices and commandBuffers associated with each window
        for (size_t i = 0; i < pdo.windows.size(); ++i)
        {
            Window* window = pdo.windows[i];
            if (window->debugLayersEnabled()) debugLayersEnabled = true;
            uint32_t imageIndex = window->nextImageIndex();
            pdo.imageIndices[i] = imageIndex;
            pdo.commandBuffers[i] = *(window->commandBuffer(imageIndex));
        }

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitDstStageMasks.data();

        submitInfo.commandBufferCount = static_cast<uint32_t>(pdo.commandBuffers.size());
        submitInfo.pCommandBuffers = pdo.commandBuffers.data();

        submitInfo.signalSemaphoreCount = static_cast<uint32_t>(pdo.signalSemaphores.size());
        submitInfo.pSignalSemaphores = pdo.signalSemaphores.data();

        if (pdo.graphicsQueue->submit(submitInfo, fence) != VK_SUCCESS)
        {
            std::cout << "Error: failed to submit draw command buffer." << std::endl;
            return false;
        }
    }

    for (auto& pair_pdo : _deviceMap)
    {
        PerDeviceObjects& pdo = pair_pdo.second;

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = static_cast<uint32_t>(pdo.signalSemaphores.size());
        presentInfo.pWaitSemaphores = pdo.signalSemaphores.data();
        presentInfo.swapchainCount = static_cast<uint32_t>(pdo.swapchains.size());
        presentInfo.pSwapchains = pdo.swapchains.data();
        presentInfo.pImageIndices = pdo.imageIndices.data();

        pdo.presentQueue->present(presentInfo);
    }

    if (debugLayersEnabled)
    {
        //auto startTime = std::chrono::steady_clock::now();

        for (auto& pair_pdo : _deviceMap)
        {
            PerDeviceObjects& pdo = pair_pdo.second;
            pdo.presentQueue->waitIdle();
        }

        //std::cout << "Viewer::submitFrame() vkQueueWaitIdle() completed in " << std::chrono::duration<double, std::chrono::milliseconds::period>(std::chrono::steady_clock::now() - startTime).count() << "ms" << std::endl;
    }

    // advance each window to the next frame
    for (auto& window : _windows)
    {
        window->advanceNextImageIndex();
    }

    return true;
}

void Viewer::compile(BufferPreferences bufferPreferences)
{

    for (auto& window : _windows)
    {
        // compile the Vulkan objects
        // create high level Vulkan objects associated the main window
        vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice(window->physicalDevice());
        vsg::ref_ptr<vsg::Device> device(window->device());

        CollectDescriptorStats collectStats;
        for (auto& stage : window->stages())
        {
            GraphicsStage* gs = dynamic_cast<GraphicsStage*>(stage.get());
            if (gs)
            {
                gs->_commandGraph->accept(collectStats);
            }
            RayTracingStage* rts = dynamic_cast<RayTracingStage*>(stage.get());
            if (rts)
            {
                rts->_commandGraph->accept(collectStats);
            }
        }

        uint32_t maxSets = collectStats.computeNumDescriptorSets();
        DescriptorPoolSizes descriptorPoolSizes = collectStats.computeDescriptorPoolSizes();

#if 0
        std::cout << "maxSlot = " << collectStats.maxSlot << std::endl;
        std::cout << "maxSets = " << maxSets << std::endl;
        std::cout << "    type\tcount" << std::endl;
        for (auto& [type, count] : descriptorPoolSizes)
        {
            std::cout << "    " << type << "\t\t" << count << std::endl;
        }
#endif
        ref_ptr<CompileTraversal> compile(new CompileTraversal(device, bufferPreferences));
        compile->context.commandPool = vsg::CommandPool::create(device, physicalDevice->getGraphicsFamily());
        compile->context.renderPass = window->renderPass();
        compile->context.graphicsQueue = device->getQueue(physicalDevice->getGraphicsFamily());

        if (maxSets > 0) compile->context.descriptorPool = vsg::DescriptorPool::create(device, maxSets, descriptorPoolSizes);

        for (auto& stage : window->stages())
        {
            GraphicsStage* gs = dynamic_cast<GraphicsStage*>(stage.get());
            RayTracingStage* rts = dynamic_cast<RayTracingStage*>(stage.get());
            if (gs)
            {
                gs->_maxSlot = collectStats.maxSlot;

                if (gs->_camera->getViewportState())
                    compile->context.viewport = gs->_camera->getViewportState();
                else if (gs->_viewport)
                    compile->context.viewport = gs->_viewport;
                else
                    compile->context.viewport = vsg::ViewportState::create(window->extent2D());

                // std::cout << "Compiling GraphicsStage " << compile.context.viewport << std::endl;

                gs->_commandGraph->accept(*compile);

                compile->context.dispatch();
                compile->context.waitForCompletion();

                if (gs->databasePager)
                {
                    gs->databasePager->compileTraversal = compile;
                    gs->databasePager->start();
                }
            }
            else if (rts)
            {
                rts->_maxSlot = collectStats.maxSlot;

                rts->_commandGraph->accept(*compile);

                compile->context.dispatch();
                compile->context.waitForCompletion();
            }
            else
            {
                std::cout << "Warning : Viewer::compile() has not handled Stage : " << stage->className() << std::endl;
            }
        }
    }
}
