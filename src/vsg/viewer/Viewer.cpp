/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>

#include <vsg/nodes/StateGroup.h>

#include <vsg/vk/Descriptor.h>

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
        auto [graphicsFamily, presentFamily] = physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT, window->surface());

        // set up per device settings
        PerDeviceObjects& new_pdo = _deviceMap[device];
        new_pdo.renderFinishedSemaphore = vsg::Semaphore::create(device);
        new_pdo.graphicsQueue = device->getQueue(graphicsFamily);
        new_pdo.presentQueue = device->getQueue(presentFamily);
        new_pdo.signalSemaphores.push_back(*new_pdo.renderFinishedSemaphore);
    }

    // add per window details to pdo
    PerDeviceObjects& pdo = _deviceMap[device];
    pdo.windows.push_back(window);
    pdo.imageIndices.push_back(0);   // to be filled in by submitFrame()
    pdo.commandBuffers.push_back(0); // to be filled in by submitFrame()
    pdo.swapchains.push_back(*(window->swapchain()));
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

        for (auto window : pdo.windows)
        {
            pdo.imageIndices.push_back(0);   // to be filled in by submitFrame()
            pdo.commandBuffers.push_back(0); // to be filled in by submitFrame()
            pdo.swapchains.push_back(*(window->swapchain()));
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

void Viewer::compile(BufferPreferences bufferPreferences)
{
    if (recordAndSubmitTasks.empty())
    {
        return;
    }

    struct DeviceResources
    {
        vsg::CollectDescriptorStats collectStats;
        vsg::ref_ptr<vsg::DescriptorPool> descriptorPool;
        vsg::ref_ptr<vsg::CompileTraversal> compile;
    };

    // find which devices are available
    using DeviceResourceMap = std::map<vsg::Device*, DeviceResources>;
    DeviceResourceMap deviceResourceMap;
    for (auto& task : recordAndSubmitTasks)
    {
        for (auto& commandGraph : task->commandGraphs)
        {
            auto& deviceResources = deviceResourceMap[commandGraph->_device];
            commandGraph->accept(deviceResources.collectStats);
        }
    }

    // allocate DescriptorPool for each Device
    for (auto& [device, deviceResource] : deviceResourceMap)
    {
        auto physicalDevice = device->getPhysicalDevice();

        auto& collectStats = deviceResource.collectStats;
        auto maxSets = std::max(1u,collectStats.computeNumDescriptorSets());
        const auto& descriptorPoolSizes =collectStats.computeDescriptorPoolSizes();

        auto queueFamily = physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT); // TODO : could we just use transfer bit?

        deviceResource.compile = new vsg::CompileTraversal(device, bufferPreferences);
        if(!descriptorPoolSizes.empty())
        deviceResource.compile->context.descriptorPool = vsg::DescriptorPool::create(device, maxSets, descriptorPoolSizes);
        deviceResource.compile->context.commandPool = vsg::CommandPool::create(device, queueFamily);
        deviceResource.compile->context.graphicsQueue = device->getQueue(queueFamily);
    }

    // create the Vulkan objects
    for (auto& task : recordAndSubmitTasks)
    {
        std::set<Device*> devices;

        for (auto& commandGraph : task->commandGraphs)
        {
            if (commandGraph->_device) devices.insert(commandGraph->_device);

            auto& deviceResource = deviceResourceMap[commandGraph->_device];
            commandGraph->_maxSlot = deviceResource.collectStats.maxSlot;
            commandGraph->accept(*deviceResource.compile);
        }

        if (task->databasePager)
        {
            // crude hack for taking first device as the one for the DatabasePager to compile resourcces for.
            for (auto& commandGraph : task->commandGraphs)
            {
                auto& deviceResource = deviceResourceMap[commandGraph->_device];
                task->databasePager->compileTraversal = deviceResource.compile;
                break;
            }
        }
    }

    // dispatch any transfer commands commands
    for (auto& dp : deviceResourceMap)
    {
        dp.second.compile->context.dispatch();
    }

    // wait for the transfers to complete
    for (auto& dp : deviceResourceMap)
    {
        dp.second.compile->context.waitForCompletion();
    }

    // start any DatabasePagers
    for (auto& task : recordAndSubmitTasks)
    {
        if (task->databasePager)
        {
            task->databasePager->start();
        }
    }
}

void Viewer::assignRecordAndSubmitTaskAndPresentation(CommandGraphs commandGraphs, DatabasePager* databasePager)
{
    if (_windows.empty()) return;

    // TODO need to resolve what to do about graphic vs non graphic operations and mulitple windows
    Window* window = _windows.front();
    Device* device = window->device();
    PhysicalDevice* physicalDevice = window->physicalDevice();

    auto [graphicsFamily, presentFamily] = physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT, window->surface());

    auto renderFinishedSemaphore = vsg::Semaphore::create(device);

    // set up Submission with CommandBuffer and signals
    auto recordAndSubmitTask = vsg::RecordAndSubmitTask::create();
    recordAndSubmitTask->commandGraphs = commandGraphs;
    recordAndSubmitTask->signalSemaphores.emplace_back(renderFinishedSemaphore);
    recordAndSubmitTask->databasePager = databasePager;
    recordAndSubmitTask->windows = _windows;
    recordAndSubmitTask->queue = device->getQueue(graphicsFamily);
    recordAndSubmitTasks.emplace_back(recordAndSubmitTask);

    presentation = vsg::Presentation::create();
    presentation->waitSemaphores.emplace_back(renderFinishedSemaphore);
    presentation->windows = _windows;
    presentation->queue = device->getQueue(presentFamily);
}

void Viewer::update()
{
    for (auto& task : recordAndSubmitTasks)
    {
        if (task->databasePager)
        {
            task->databasePager->updateSceneGraph(_frameStamp);
        }
    }
}

void Viewer::recordAndSubmit()
{
    for (auto& recordAndSubmitTask : recordAndSubmitTasks)
    {
        recordAndSubmitTask->submit(_frameStamp);
    }
}

void Viewer::present()
{
    if (presentation) presentation->present();
}
