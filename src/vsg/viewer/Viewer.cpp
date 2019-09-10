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

    if (!aquireNextFrame()) return false;

    // create FrameStamp for frame
    auto time = vsg::clock::now();
    _frameStamp = _frameStamp ? new vsg::FrameStamp(time, _frameStamp->frameCount + 1) : new vsg::FrameStamp(time, 0);

    // create an event for the new frame.
    _events.emplace_back(new FrameEvent(_frameStamp));

    return true;
}

bool Viewer::aquireNextFrame()
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
                vkQueueWaitIdle(pdo.presentQueue);
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
        window->populateCommandBuffers(window->nextImageIndex());
    }
    return true;
}

bool Viewer::submitNextFrame()
{
    bool debugLayersEnabled = false;

    for (auto& pair_pdo : _deviceMap)
    {
        PerDeviceObjects& pdo = pair_pdo.second;

        VkFence fence = VK_NULL_HANDLE;

        std::vector<VkSemaphore> waitSemaphores;
        for (auto& window : pdo.windows)
        {
            waitSemaphores.push_back(*(window->frame(window->nextImageIndex()).imageAvailableSemaphore));
            fence = *(window->frame(window->nextImageIndex()).commandsCompletedFence);
            window->frame(window->nextImageIndex()).checkCommandsCompletedFence = true;
        }

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
        submitInfo.pWaitDstStageMask = pdo.waitStages.data();

        submitInfo.commandBufferCount = static_cast<uint32_t>(pdo.commandBuffers.size());
        submitInfo.pCommandBuffers = pdo.commandBuffers.data();

        submitInfo.signalSemaphoreCount = static_cast<uint32_t>(pdo.signalSemaphores.size());
        submitInfo.pSignalSemaphores = pdo.signalSemaphores.data();

        if (vkQueueSubmit(pdo.graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS)
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

        vkQueuePresentKHR(pdo.presentQueue, &presentInfo);
    }

    if (debugLayersEnabled)
    {
        //auto startTime = std::chrono::steady_clock::now();

        for (auto& pair_pdo : _deviceMap)
        {
            PerDeviceObjects& pdo = pair_pdo.second;
            vkQueueWaitIdle(pdo.presentQueue);
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

class CollectDescriptorStats : public ConstVisitor
{
public:
    using Descriptors = std::set<const Descriptor*>;
    using DescriptorSets = std::set<const DescriptorSet*>;
    using DescriptorTypeMap = std::map<VkDescriptorType, uint32_t>;

    using ConstVisitor::apply;

    void apply(const Object& object) override
    {
        object.traverse(*this);
    }

    void apply(const StateGroup& stategroup) override
    {
        for (auto& command : stategroup.getStateCommands())
        {
            command->accept(*this);
        }

        stategroup.traverse(*this);
    }

    void apply(const StateCommand& stateCommand) override
    {
        if (stateCommand.getSlot() > maxSlot) maxSlot = stateCommand.getSlot();

        stateCommand.traverse(*this);
    }
    void apply(const DescriptorSet& descriptorSet) override
    {
        if (descriptorSets.count(&descriptorSet) == 0)
        {
            descriptorSets.insert(&descriptorSet);
            for (auto& descriptor : descriptorSet.getDescriptors())
            {
                apply(*descriptor);
            }
        }
    }

    void apply(const Descriptor& descriptor)
    {
        if (descriptors.count(&descriptor) == 0)
        {
            descriptors.insert(&descriptor);
        }
        descriptorTypeMap[descriptor._descriptorType] += descriptor.getNumDescriptors();
    }

    uint32_t computeNumDescriptorSets() const
    {
        return static_cast<uint32_t>(descriptorSets.size());
    }

    DescriptorPoolSizes computeDescriptorPoolSizes() const
    {
        DescriptorPoolSizes poolSizes;
        for (auto& [type, count] : descriptorTypeMap)
        {
            poolSizes.push_back(VkDescriptorPoolSize{type, count});
        }
        return poolSizes;
    }

    Descriptors descriptors;
    DescriptorSets descriptorSets;
    DescriptorTypeMap descriptorTypeMap;
    uint32_t maxSlot = 0;
};

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
                if (gs->_offscreenPreRenderPass) gs->_offscreenPreRenderPass->_commandGraph->accept(collectStats);
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
        vsg::CompileTraversal compile(device, bufferPreferences);
        compile.context.commandPool = vsg::CommandPool::create(device, physicalDevice->getGraphicsFamily());
        compile.context.renderPass = window->renderPass();
        compile.context.graphicsQueue = device->getQueue(physicalDevice->getGraphicsFamily());

        if (maxSets > 0) compile.context.descriptorPool = vsg::DescriptorPool::create(device, maxSets, descriptorPoolSizes);

        for (auto& stage : window->stages())
        {
            GraphicsStage* gs = dynamic_cast<GraphicsStage*>(stage.get());
            if (gs)
            {
                gs->_maxSlot = collectStats.maxSlot;

                if (gs->_camera->getViewportState())
                    compile.context.viewport = gs->_camera->getViewportState();
                else if (gs->_viewport)
                    compile.context.viewport = gs->_viewport;
                else
                    compile.context.viewport = vsg::ViewportState::create(window->extent2D());

                // std::cout << "Compiling GraphicsStage " << compile.context.viewport << std::endl;

                gs->_commandGraph->accept(compile);

                if (gs->_offscreenPreRenderPass)
                {
                    //HACK, switch the compile context render pass and viewport
                    compile.context.renderPass = gs->_offscreenPreRenderPass->_renderPass;

                    if (gs->_offscreenPreRenderPass->_camera->getViewportState())
                        compile.context.viewport = gs->_offscreenPreRenderPass->_camera->getViewportState();
                    else if (gs->_offscreenPreRenderPass->_viewport)
                        compile.context.viewport = gs->_offscreenPreRenderPass->_viewport;

                    gs->_offscreenPreRenderPass->_commandGraph->accept(compile);

                    compile.context.renderPass = window->renderPass();
                }

                compile.context.dispatchCommands();
            }
            else
            {
                std::cout << "Warning : Viewer::compile() has not handled Stage : " << stage->className() << std::endl;
            }
        }
    }
}
