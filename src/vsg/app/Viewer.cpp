/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/CompileTraversal.h>
#include <vsg/app/View.h>
#include <vsg/app/Viewer.h>
#include <vsg/io/Logger.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/Descriptor.h>

#include <chrono>
#include <map>
#include <set>

using namespace vsg;

#if VK_HEADER_VERSION < 106
#    define VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT VkResult(-1000255000)
#endif

Viewer::Viewer() :
    updateOperations(UpdateOperations::create()),
    status(vsg::ActivityStatus::create()),
    _start_point(clock::now())
{
}

Viewer::~Viewer()
{
    stopThreading();

    // don't destroy viewer while devices are still active
    Viewer::deviceWaitIdle();
}

void Viewer::deviceWaitIdle() const
{
    std::set<VkDevice> devices;
    for (auto& window : _windows)
    {
        if (window->getDevice()) devices.insert(*(window->getDevice()));
    }

    for (auto& task : recordAndSubmitTasks)
    {
        for (auto& cg : task->commandGraphs)
        {
            devices.insert(*(cg->device));
        }
    }

    for (auto& device : devices)
    {
        vkDeviceWaitIdle(device);
    }
}

void Viewer::addWindow(ref_ptr<Window> window)
{
    _windows.push_back(window);
}

void Viewer::close()
{
    _close = true;
    status->set(false);

    stopThreading();
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
        deviceWaitIdle();
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

bool Viewer::advanceToNextFrame()
{
    if (!active()) return false;

    // poll all the windows for events.
    pollEvents(true);

    if (!acquireNextFrame()) return false;

    // create FrameStamp for frame
    auto time = vsg::clock::now();
    if (!_frameStamp)
    {
        // first frame, initialize to frame count and indices to 0
        _frameStamp = FrameStamp::create(time, 0);

        for (auto& task : recordAndSubmitTasks)
        {
            task->advance();
        }
    }
    else
    {
        // after first frame so increment frame count and indices
        _frameStamp = FrameStamp::create(time, _frameStamp->frameCount + 1);

        for (auto& task : recordAndSubmitTasks)
        {
            task->advance();
        }
    }

    // create an event for the new frame.
    _events.emplace_back(new FrameEvent(_frameStamp));

    return true;
}

bool Viewer::acquireNextFrame()
{
    if (_close) return false;

    VkResult result = VK_SUCCESS;

    for (auto& window : _windows)
    {
        if (!window->visible()) continue;

        while ((result = window->acquireNextImage()) != VK_SUCCESS)
        {
            if (result == VK_ERROR_SURFACE_LOST_KHR ||
                result == VK_ERROR_DEVICE_LOST ||
                result == VK_ERROR_OUT_OF_DATE_KHR ||
                result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT ||
                result == VK_SUBOPTIMAL_KHR)
            {
                // force a rebuild of the Swapchain by calling Window::resize();
                window->resize();
            }
            else
            {
                warn("window->acquireNextImage() VkResult = ", result);
                break;
            }
        }
    }

    return result == VK_SUCCESS;
}

VkResult Viewer::waitForFences(size_t relativeFrameIndex, uint64_t timeout)
{
    VkResult result = VK_SUCCESS;
    for (auto& task : recordAndSubmitTasks)
    {
        auto fenceToWait = task->fence(relativeFrameIndex);
        if (fenceToWait)
        {
            result = fenceToWait->wait(timeout);
            if (result != VK_SUCCESS) return result;
        }
    }
    return result;
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

void Viewer::compile(ref_ptr<ResourceHints> hints)
{
    if (recordAndSubmitTasks.empty())
    {
        return;
    }

    if (!compileManager) compileManager = CompileManager::create(*this, hints);

    auto start_tick = clock::now();

    bool containsPagedLOD = false;
    ref_ptr<DatabasePager> databasePager;

    struct DeviceResources
    {
        CollectResourceRequirements collectResources;
        vsg::ref_ptr<vsg::CompileTraversal> compile;
    };

    // find which devices are available and the resources required for then,
    using DeviceResourceMap = std::map<ref_ptr<vsg::Device>, DeviceResources>;
    DeviceResourceMap deviceResourceMap;
    for (auto& task : recordAndSubmitTasks)
    {
        auto& collectResources = deviceResourceMap[task->device].collectResources;
        for (auto& commandGraph : task->commandGraphs)
        {

            commandGraph->accept(collectResources);
        }

        if (task->databasePager && !databasePager) databasePager = task->databasePager;
    }

    // allocate DescriptorPool for each Device
    ResourceRequirements::Views views;
    for (auto& [device, deviceResources] : deviceResourceMap)
    {
        auto& collectResources = deviceResources.collectResources;
        auto& resourceRequirements = collectResources.requirements;

        if (hints) hints->accept(collectResources);

        views.insert(resourceRequirements.views.begin(), resourceRequirements.views.end());

        if (resourceRequirements.containsPagedLOD) containsPagedLOD = true;

        auto physicalDevice = device->getPhysicalDevice();

        auto queueFamily = physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT); // TODO : could we just use transfer bit?

        deviceResources.compile = CompileTraversal::create(device, resourceRequirements);

        for (auto& context : deviceResources.compile->contexts)
        {
            context->commandPool = vsg::CommandPool::create(device, queueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
            context->graphicsQueue = device->getQueue(queueFamily);

            context->reserve(resourceRequirements);
        }
    }

    // assign the viewID's to each View
    for (auto& [const_view, binDetails] : views)
    {
        auto view = const_cast<View*>(const_view);
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
                Bin::SortOrder sortOrder = (binNumber < 0) ? Bin::ASCENDING : ((binNumber == 0) ? Bin::NO_SORT : Bin::DESCENDING);
                view->bins.push_back(Bin::create(binNumber, sortOrder));
            }
        }
    }

    if (containsPagedLOD && !databasePager) databasePager = DatabasePager::create();

    // create the Vulkan objects
    for (auto& task : recordAndSubmitTasks)
    {
        auto& deviceResource = deviceResourceMap[task->device];
        auto& resourceRequirements = deviceResource.collectResources.requirements;

        bool task_containsPagedLOD = false;

        for (auto& commandGraph : task->commandGraphs)
        {
            commandGraph->maxSlot = resourceRequirements.maxSlot;
            commandGraph->accept(*deviceResource.compile);

            if (resourceRequirements.containsPagedLOD) task_containsPagedLOD = true;
        }

        if (task_containsPagedLOD)
        {
            if (!task->databasePager) task->databasePager = databasePager;
        }

        if (task->databasePager)
        {
            task->databasePager->compileManager = compileManager;
        }

        if (task->earlyTransferTask)
        {
            task->earlyTransferTask->assign(resourceRequirements.earlyDynamicData);
        }
        if (task->lateTransferTask)
        {
            task->lateTransferTask->assign(resourceRequirements.lateDynamicData);
        }
    }

    // record any transfer commands
    for (auto& dp : deviceResourceMap)
    {
        dp.second.compile->record();
    }

    // wait for the transfers to complete
    for (auto& dp : deviceResourceMap)
    {
        dp.second.compile->waitForCompletion();
    }

    // start any DatabasePagers
    for (auto& task : recordAndSubmitTasks)
    {
        if (task->databasePager)
        {
            task->databasePager->start();
        }
    }

    auto end_tick = clock::now();
    auto compile_time = std::chrono::duration<double, std::chrono::milliseconds::period>(end_tick - start_tick).count();
    debug("Viewer::compile() ", compile_time, "ms");
}

void Viewer::assignRecordAndSubmitTaskAndPresentation(CommandGraphs in_commandGraphs)
{
    struct DeviceQueueFamily
    {
        Device* device = nullptr;
        int queueFamily = -1;
        int presentFamily = -1;

        bool operator<(const DeviceQueueFamily& rhs) const
        {
            if (device < rhs.device) return true;
            if (device > rhs.device) return false;
            if (queueFamily < rhs.queueFamily) return true;
            if (queueFamily > rhs.queueFamily) return false;
            return presentFamily < rhs.presentFamily;
        }
    };

    // place the input CommandGraphs into separate groups associated with each device and queue family combination
    std::map<DeviceQueueFamily, CommandGraphs> deviceCommandGraphsMap;
    for (auto& commandGraph : in_commandGraphs)
    {
        deviceCommandGraphsMap[DeviceQueueFamily{commandGraph->device.get(), commandGraph->queueFamily, commandGraph->presentFamily}].emplace_back(commandGraph);
    }

    // create the required RecordAndSubmitTask and any Presentation objects that are required for each set of CommandGraphs
    for (auto& [deviceQueueFamily, commandGraphs] : deviceCommandGraphsMap)
    {
        // make sure the secondary CommandGraphs appear first in the commandGraphs list so they are filled in first
        CommandGraphs primary_commandGraphs;
        CommandGraphs secondary_commandGraphs;
        for (auto& commandGraph : commandGraphs)
        {
            if (commandGraph->level() == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
                primary_commandGraphs.emplace_back(commandGraph);
            else
                secondary_commandGraphs.emplace_back(commandGraph);
        }
        if (!secondary_commandGraphs.empty())
        {
            commandGraphs = secondary_commandGraphs;
            commandGraphs.insert(commandGraphs.end(), primary_commandGraphs.begin(), primary_commandGraphs.end());
        }

        uint32_t numBuffers = 3;

        auto device = deviceQueueFamily.device;
        uint32_t transferQueueFamily = device->getPhysicalDevice()->getQueueFamily(VK_QUEUE_TRANSFER_BIT);

        if (deviceQueueFamily.presentFamily >= 0)
        {
            // collate all the unique Windows associated with these commandGraphs
            std::set<Window*> uniqueWindows;
            for (auto& commanGraph : commandGraphs)
            {
                uniqueWindows.insert(commanGraph->window);
            }

            Windows windows(uniqueWindows.begin(), uniqueWindows.end());

            auto renderFinishedSemaphore = vsg::Semaphore::create(device);

            // set up Submission with CommandBuffer and signals
            auto recordAndSubmitTask = vsg::RecordAndSubmitTask::create(device, numBuffers);
            recordAndSubmitTask->commandGraphs = commandGraphs;
            recordAndSubmitTask->signalSemaphores.emplace_back(renderFinishedSemaphore);
            recordAndSubmitTask->windows = windows;
            recordAndSubmitTask->queue = device->getQueue(deviceQueueFamily.queueFamily);
            recordAndSubmitTasks.emplace_back(recordAndSubmitTask);

            recordAndSubmitTask->earlyTransferTask->transferQueue = device->getQueue(transferQueueFamily);
            recordAndSubmitTask->lateTransferTask->transferQueue = device->getQueue(transferQueueFamily);

            auto presentation = vsg::Presentation::create();
            presentation->waitSemaphores.emplace_back(renderFinishedSemaphore);
            presentation->windows = windows;
            presentation->queue = device->getQueue(deviceQueueFamily.presentFamily);
            presentations.emplace_back(presentation);
        }
        else
        {
            // with don't have a presentFamily so this set of commandGraphs aren't associated with a window
            // set up Submission with CommandBuffer and signals
            auto recordAndSubmitTask = vsg::RecordAndSubmitTask::create(device, numBuffers);
            recordAndSubmitTask->commandGraphs = commandGraphs;
            recordAndSubmitTask->queue = device->getQueue(deviceQueueFamily.queueFamily);
            recordAndSubmitTasks.emplace_back(recordAndSubmitTask);

            recordAndSubmitTask->earlyTransferTask->transferQueue = device->getQueue(transferQueueFamily);
            recordAndSubmitTask->lateTransferTask->transferQueue = device->getQueue(transferQueueFamily);
        }
    }
}

void Viewer::setupThreading()
{
    debug("Viewer::setupThreading() ");

    stopThreading();

    // check how valid tasks and command graphs there are.
    uint32_t numValidTasks = 0;
    size_t numCommandGraphs = 0;
    size_t numEarlyTransferTasks = 0;
    for (auto& task : recordAndSubmitTasks)
    {
        if (!task->commandGraphs.empty())
        {
            ++numValidTasks;
            numCommandGraphs += task->commandGraphs.size();
            if (task->earlyTransferTask) ++numEarlyTransferTasks;
        }
    }

    // check if there is any point in setting up threading
    if (numValidTasks == 0)
    {
        return;
    }

    status->set(true);
    _threading = true;
    _frameBlock = FrameBlock::create(status);
    _submissionCompleted = Barrier::create(1 + numValidTasks);

    // set up required threads for each task
    for (auto& task : recordAndSubmitTasks)
    {
        if (task->commandGraphs.size() == 1 && !task->earlyTransferTask)
        {
            // task only contains a single CommandGraph so keep thread simple
            auto run = [](ref_ptr<RecordAndSubmitTask> viewer_task, ref_ptr<FrameBlock> viewer_frameBlock, ref_ptr<Barrier> submissionCompleted) {
                auto frameStamp = viewer_frameBlock->initial_value;

                // wait for this frame to be signaled
                while (viewer_frameBlock->wait_for_change(frameStamp))
                {
                    viewer_task->submit(frameStamp);

                    submissionCompleted->arrive_and_drop();
                }
            };

            threads.emplace_back(run, task, _frameBlock, _submissionCompleted);
        }
        else if (!task->commandGraphs.empty())
        {
            // we have multiple CommandGraphs in a single Task so set up a thread per CommandGraph
            struct SharedData : public Inherit<Object, SharedData>
            {
                SharedData(ref_ptr<RecordAndSubmitTask> in_task, ref_ptr<FrameBlock> in_frameBlock, ref_ptr<Barrier> in_submissionCompleted, uint32_t numThreads) :
                    task(in_task),
                    frameBlock(in_frameBlock),
                    submissionCompletedBarrier(in_submissionCompleted)
                {
                    recordStartBarrier = Barrier::create(numThreads);
                    recordCompletedBarrier = Barrier::create(numThreads);
                }

                void add(CommandBuffers& commandBuffers)
                {
                    std::scoped_lock lock(recordCommandBuffersMutex);
                    recordedCommandBuffers.insert(recordedCommandBuffers.end(), commandBuffers.begin(), commandBuffers.end());
                }

                // shared between all threads
                ref_ptr<RecordAndSubmitTask> task;
                ref_ptr<FrameBlock> frameBlock;
                ref_ptr<Barrier> submissionCompletedBarrier;

                // shared between threads associated with each task
                std::mutex recordCommandBuffersMutex;
                CommandBuffers recordedCommandBuffers;

                ref_ptr<Barrier> recordStartBarrier;
                ref_ptr<Barrier> recordCompletedBarrier;
            };

            uint32_t numThreads = static_cast<uint32_t>(task->commandGraphs.size());
            if (task->earlyTransferTask) ++numThreads;

            ref_ptr<SharedData> sharedData = SharedData::create(task, _frameBlock, _submissionCompleted, numThreads);

            auto run_primary = [](ref_ptr<SharedData> data, ref_ptr<CommandGraph> commandGraph) {
                auto frameStamp = data->frameBlock->initial_value;

                // wait for this frame to be signaled
                while (data->frameBlock->wait_for_change(frameStamp))
                {
                    // primary thread starts the task
                    data->task->start();

                    data->recordStartBarrier->arrive_and_wait();

                    //vsg::info("run_primary");

                    CommandBuffers localRecordedCommandBuffers;
                    commandGraph->record(localRecordedCommandBuffers, frameStamp, data->task->databasePager);

                    data->add(localRecordedCommandBuffers);

                    data->recordCompletedBarrier->arrive_and_wait();

                    // primary thread finishes the task, submitting all the command buffers recorded by the primary and all secondary threads to it's queue
                    data->task->finish(data->recordedCommandBuffers);
                    data->recordedCommandBuffers.clear();

                    data->submissionCompletedBarrier->arrive_and_wait();
                }
            };

            auto run_secondary = [](ref_ptr<SharedData> data, ref_ptr<CommandGraph> commandGraph) {
                auto frameStamp = data->frameBlock->initial_value;

                // wait for this frame to be signaled
                while (data->frameBlock->wait_for_change(frameStamp))
                {
                    data->recordStartBarrier->arrive_and_wait();

                    CommandBuffers localRecordedCommandBuffers;
                    commandGraph->record(localRecordedCommandBuffers, frameStamp, data->task->databasePager);

                    data->add(localRecordedCommandBuffers);

                    data->recordCompletedBarrier->arrive_and_wait();
                }
            };

            auto run_transfer = [](ref_ptr<SharedData> data, ref_ptr<TransferTask> transferTask) {
                auto frameStamp = data->frameBlock->initial_value;

                // wait for this frame to be signaled
                while (data->frameBlock->wait_for_change(frameStamp))
                {
                    data->recordStartBarrier->arrive_and_wait();

                    //vsg::info("run_transfer");

                    /*VkResult result =*/transferTask->transferDynamicData();

                    data->recordCompletedBarrier->arrive_and_wait();
                }
            };

            for (uint32_t i = 0; i < task->commandGraphs.size(); ++i)
            {
                if (i == 0)
                    threads.emplace_back(run_primary, sharedData, task->commandGraphs[i]);
                else
                    threads.emplace_back(run_secondary, sharedData, task->commandGraphs[i]);
            }

            if (task->earlyTransferTask)
            {
                threads.emplace_back(run_transfer, sharedData, task->earlyTransferTask);
            }
        }
    }
}

void Viewer::stopThreading()
{
    if (!_threading) return;
    _threading = false;

    debug("Viewer::stopThreading()");

    // release the blocks to enable threads to exit cleanly
    // need to manually wake up the threads waiting on this frameBlock so they check the status value and exit cleanly.
    status->set(false);
    _frameBlock->wake();

    for (auto& thread : threads)
    {
        if (thread.joinable()) thread.join();
    }
    threads.clear();
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

    updateOperations->run();
}

void Viewer::recordAndSubmit()
{
    // reset connected ExecuteCommands
    for (auto& recordAndSubmitTask : recordAndSubmitTasks)
    {
        for (auto& commandGraph : recordAndSubmitTask->commandGraphs)
        {
            commandGraph->reset();
        }
    }

#if 1
    if (_threading)
#else
    // follows is a workaround for an odd "Possible data race during write of size 1" warning that valgrind tool=helgrind reports
    // on the first call to vkBeginCommandBuffer despite them being done on independent command buffers.  This could well be a driver bug or a false position.
    // if you want to quiet this warning then change the #if above to #if 0 as render the first three frames single threaded avoids the warning.
    if (_threading && _frameStamp->frameCount > 2)
#endif
    {
        _frameBlock->set(_frameStamp);
        _submissionCompleted->arrive_and_wait();
    }
    else
    {
        for (auto& recordAndSubmitTask : recordAndSubmitTasks)
        {
            recordAndSubmitTask->submit(_frameStamp);
        }
    }
}

void Viewer::present()
{
    for (auto& presentation : presentations)
    {
        presentation->present();
    }
}

void vsg::updateViewer(Viewer& viewer, const CompileResult& compileResult)
{
    updateTasks(viewer.recordAndSubmitTasks, viewer.compileManager, compileResult);
}
