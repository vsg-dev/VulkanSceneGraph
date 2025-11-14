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
    animationManager(AnimationManager::create()),
    status(vsg::ActivityStatus::create()),
    _firstFrame(true),
    _start_point(clock::now()),
    _frameStamp(FrameStamp::create(_start_point, 0, 0.0))
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer constructor", COLOR_VIEWER);
}

Viewer::~Viewer()
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer destructor", COLOR_VIEWER);

    stopThreading();

    // don't destroy viewer while devices are still active
    Viewer::deviceWaitIdle();
}

void Viewer::deviceWaitIdle() const
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer deviceWaitIdle", COLOR_VIEWER);

    std::set<VkDevice> devices;
    for (auto& window : _windows)
    {
        if (window->getDevice()) devices.insert(*(window->getDevice()));
    }

    for (const auto& task : recordAndSubmitTasks)
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
    // make sure the addition is unique
    auto itr = std::find(_windows.begin(), _windows.end(), window);
    if (itr != _windows.end()) return;

    _windows.push_back(window);
}

void Viewer::removeWindow(ref_ptr<Window> window)
{
    auto itr = std::find(_windows.begin(), _windows.end(), window);
    if (itr == _windows.end()) return;

    _windows.erase(itr);

    // create a new list of CommandGraphs not associated with removed window
    CommandGraphs commandGraphs;
    for (const auto& task : recordAndSubmitTasks)
    {
        for (auto& cg : task->commandGraphs)
        {
            if (cg->window != window) commandGraphs.push_back(cg);
        }
    }

    // assign the remaining commandGraphs
    assignRecordAndSubmitTaskAndPresentation(commandGraphs);
}

void Viewer::close()
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer close", COLOR_VIEWER);

    _close = true;
    status->set(false);

    stopThreading();
}

bool Viewer::active() const
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer active", COLOR_VIEWER);

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
        // don't exit mainloop while any devices are still active
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
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer pollEvents", COLOR_UPDATE);

    bool result = false;

    if (discardPreviousEvents) _events.clear();
    for (auto& window : _windows)
    {
        if (window->pollEvents(_events)) result = true;
    }

    return result;
}

bool Viewer::advanceToNextFrame(double simulationTime)
{
    static constexpr SourceLocation s_frame_source_location{"Viewer advanceToNextFrame", VsgFunctionName, __FILE__, __LINE__, COLOR_VIEWER, 1};

    // signal to instrumentation the end of the previous frame
    if (instrumentation && _frameStamp) instrumentation->leaveFrame(&s_frame_source_location, frameReference, *_frameStamp);

    if (!active())
    {
        return false;
    }

    // poll all the windows for events.
    pollEvents(true);

    if (!acquireNextFrame()) return false;

    // create FrameStamp for frame
    auto time = vsg::clock::now();
    if (_firstFrame)
    {
        _firstFrame = false;

        if (simulationTime == UseTimeSinceStartPoint) simulationTime = 0.0;

        // first frame, initialize to frame count and indices to 0
        _frameStamp = FrameStamp::create(time, 0, simulationTime);
    }
    else
    {
        // after first frame so increment frame count and indices
        if (simulationTime == UseTimeSinceStartPoint)
        {
            simulationTime = std::chrono::duration<double, std::chrono::seconds::period>(time - _start_point).count();
        }

        _frameStamp = FrameStamp::create(time, _frameStamp->frameCount + 1, simulationTime);
    }

    // signal to instrumentation the start of frame
    if (instrumentation) instrumentation->enterFrame(&s_frame_source_location, frameReference, *_frameStamp);

    for (auto& task : recordAndSubmitTasks)
    {
        task->advance();
    }

    // create an event for the new frame.
    _events.emplace_back(new FrameEvent(_frameStamp));

    return true;
}

bool Viewer::acquireNextFrame()
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer acquireNextFrame", COLOR_VIEWER);

    if (_close) return false;

    VkResult result = VK_SUCCESS;

    for (auto& window : _windows)
    {
        if (!window->visible()) continue;

        while ((result = window->acquireNextImage()) != VK_SUCCESS)
        {
            if (result == VK_ERROR_SURFACE_LOST_KHR ||
                result == VK_ERROR_OUT_OF_DATE_KHR ||
                result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT ||
                result == VK_SUBOPTIMAL_KHR)
            {
                // force a rebuild of the Swapchain by calling Window::resize();
                window->resize();
            }
            else if (result == VK_ERROR_DEVICE_LOST)
            {
                // a lost device can only be recovered by opening a new VkDevice, and success is not guaranteed.
                // not currently implemented, so exit main loop.
                warn("window->acquireNextImage() VkResult = VK_ERROR_DEVICE_LOST. Device loss can indicate invalid Vulkan API usage or driver/hardware issues.");
                break;
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
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer waitForFences", COLOR_VIEWER);

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
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer handle events", COLOR_UPDATE);

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
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer compile", COLOR_COMPILE);

    if (recordAndSubmitTasks.empty())
    {
        return;
    }

    bool containsPagedLOD = false;
    ref_ptr<DatabasePager> databasePager;

    struct DeviceResources
    {
        CollectResourceRequirements collectResources;
    };

    // find which devices are available and the resources required for them
    using DeviceResourceMap = std::map<ref_ptr<vsg::Device>, DeviceResources>;
    DeviceResourceMap deviceResourceMap;
    for (auto& task : recordAndSubmitTasks)
    {
        auto& collectResources = deviceResourceMap[task->device].collectResources;
        auto& resourceRequirements = collectResources.requirements;
        if (hints) hints->accept(collectResources);

        for (auto& commandGraph : task->commandGraphs)
        {
            commandGraph->accept(collectResources);
        }

        task->transferTask->minimumStagingBufferSize = resourceRequirements.minimumStagingBufferSize;

        if (task->databasePager && !databasePager) databasePager = task->databasePager;
    }

    // allocate DescriptorPool for each Device
    ResourceRequirements::Views views;
    for (auto& [device, deviceResources] : deviceResourceMap)
    {
        auto& collectResources = deviceResources.collectResources;
        auto& resourceRequirements = collectResources.requirements;

        views.insert(resourceRequirements.views.begin(), resourceRequirements.views.end());

        if (resourceRequirements.containsPagedLOD) containsPagedLOD = true;
    }

    // assign the viewID's to each View
    for (auto& [const_view, binDetails] : views)
    {
        auto view = const_cast<View*>(const_view);
        for (auto& binNumber : binDetails.indices)
        {
            bool binNumberMatched = false;
            for (const auto& bin : view->bins)
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

    if (containsPagedLOD && !databasePager)
    {
        databasePager = DatabasePager::create();
        if (instrumentation) databasePager->assignInstrumentation(instrumentation);
    }

    // create the Vulkan objects
    for (const auto& task : recordAndSubmitTasks)
    {
        const auto& deviceResource = deviceResourceMap[task->device];
        const auto& resourceRequirements = deviceResource.collectResources.requirements;

        bool task_containsPagedLOD = false;

        for (const auto& commandGraph : task->commandGraphs)
        {
            commandGraph->maxSlots = resourceRequirements.maxSlots;
            if (resourceRequirements.containsPagedLOD) task_containsPagedLOD = true;
        }

        if (task_containsPagedLOD)
        {
            if (!task->databasePager) task->databasePager = databasePager;
        }
    }

    // set up the CompileManager
    if (!compileManager)
    {
        compileManager = CompileManager::create(*this, hints);
        if (instrumentation) compileManager->assignInstrumentation(instrumentation);
    }

    // assign CompileManager to DatabasePager
    if (databasePager && !databasePager->compileManager)
    {
        databasePager->compileManager = compileManager;
    }

    for (auto& task : recordAndSubmitTasks)
    {
        auto& deviceResource = deviceResourceMap[task->device];
        auto& resourceRequirements = deviceResource.collectResources.requirements;
        compileManager->compileTask(task, resourceRequirements);
        task->transferTask->assign(resourceRequirements.dynamicData);
    }

    // start any DatabasePagers
    for (const auto& task : recordAndSubmitTasks)
    {
        if (task->databasePager)
        {
            if (hints)
                task->databasePager->start(hints->numDatabasePagerReadThreads);
            else
                task->databasePager->start();
        }
    }
}

void Viewer::assignRecordAndSubmitTaskAndPresentation(CommandGraphs in_commandGraphs)
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer assignRecordAndSubmitTaskAndPresentation", COLOR_VIEWER);

    // now remove any commandGraphs associated with window
    bool needToStartThreading = _threading;
    if (_threading) stopThreading();

    // if a DatabasePager is already assigned re-assign
    ref_ptr<DatabasePager> databasePager;
    for (const auto& task : recordAndSubmitTasks)
    {
        if (task->databasePager)
        {
            databasePager = task->databasePager;
            break;
        }
    }

    presentations.clear();
    recordAndSubmitTasks.clear();

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

    // find all the windows
    struct FindWindows : public Visitor
    {
        std::set<ref_ptr<Window>> windows;

        void apply(Object& object) override { object.traverse(*this); }
        void apply(CommandGraph& cg) override
        {
            if (cg.window) windows.insert(cg.window);
            cg.traverse(*this);
        }
    } findWindows;

    // place the input CommandGraphs into separate groups associated with each device and queue family combination
    std::map<DeviceQueueFamily, CommandGraphs> deviceCommandGraphsMap;
    for (auto& commandGraph : in_commandGraphs)
    {
        commandGraph->accept(findWindows);
        deviceCommandGraphsMap[DeviceQueueFamily{commandGraph->device.get(), commandGraph->queueFamily, commandGraph->presentFamily}].emplace_back(commandGraph);
    }

    // assign the windows found in the CommandGraphs so that the Viewer can track them.
    _windows.assign(findWindows.windows.begin(), findWindows.windows.end());

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

        // get main queue used for RecordAndSubmitTask
        ref_ptr<Queue> mainQueue = device->getQueue(deviceQueueFamily.queueFamily);

        // get presentation queue if required/supported
        ref_ptr<Queue> presentQueue;
        if (deviceQueueFamily.presentFamily >= 0) presentQueue = device->getQueue(deviceQueueFamily.presentFamily);

        // get an appropriate transfer queue
        ref_ptr<Queue> transferQueue = mainQueue;

        VkQueueFlags transferQueueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT; // use VK_QUEUE_GRAPHICS_BIT to ensure we can blit images
        for (auto& queue : device->getQueues())
        {
            if ((queue->queueFlags() & transferQueueFlags) == transferQueueFlags)
            {
                if (queue != mainQueue)
                {
                    transferQueue = queue;
                    break;
                }
            }
        }

        if (deviceQueueFamily.presentFamily >= 0)
        {
            // collate all the unique Windows associated with this device's commandGraphs
            findWindows.windows.clear();
            for (auto& commandGraph : commandGraphs)
            {
                commandGraph->accept(findWindows);
            }

            Windows activeWindows(findWindows.windows.begin(), findWindows.windows.end());

            // set up Submission with CommandBuffer and signals
            auto recordAndSubmitTask = vsg::RecordAndSubmitTask::create(device, numBuffers);
            recordAndSubmitTask->commandGraphs = commandGraphs;
            recordAndSubmitTask->databasePager = databasePager;
            recordAndSubmitTask->windows = activeWindows;
            recordAndSubmitTask->queue = mainQueue;
            recordAndSubmitTasks.emplace_back(recordAndSubmitTask);

            recordAndSubmitTask->transferTask->transferQueue = transferQueue;

            // assign instrumentation
            if (instrumentation) recordAndSubmitTask->assignInstrumentation(instrumentation);

            auto presentation = vsg::Presentation::create();
            presentation->windows = activeWindows;
            presentation->queue = device->getQueue(deviceQueueFamily.presentFamily);
            presentations.emplace_back(presentation);
        }
        else
        {
            // we don't have a presentFamily so this set of commandGraphs aren't associated with a window
            // set up Submission with CommandBuffer and signals
            auto recordAndSubmitTask = vsg::RecordAndSubmitTask::create(device, numBuffers);
            recordAndSubmitTask->commandGraphs = commandGraphs;
            recordAndSubmitTask->databasePager = databasePager;
            recordAndSubmitTask->queue = mainQueue;
            recordAndSubmitTasks.emplace_back(recordAndSubmitTask);

            recordAndSubmitTask->transferTask->transferQueue = transferQueue;

            // assign instrumentation
            if (instrumentation) recordAndSubmitTask->assignInstrumentation(instrumentation);
        }
    }

    if (needToStartThreading) setupThreading();
}

void Viewer::addRecordAndSubmitTaskAndPresentation(CommandGraphs commandGraphs)
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer addRecordAndSubmitTaskAndPresentation", COLOR_VIEWER);

    // collect the existing CommandGraphs
    CommandGraphs combinedCommandGraphs;
    for (const auto& task : recordAndSubmitTasks)
    {
        for (auto& cg : task->commandGraphs)
        {
            combinedCommandGraphs.push_back(cg);
        }
    }

    // add the new CommandGraphs
    combinedCommandGraphs.insert(combinedCommandGraphs.end(), commandGraphs.begin(), commandGraphs.end());

    // assign the combined CommandGraphs
    assignRecordAndSubmitTaskAndPresentation(combinedCommandGraphs);
}

void Viewer::setupThreading()
{
    stopThreading();

    // check how many valid tasks there are.
    uint32_t numValidTasks = 0;
    for (const auto& task : recordAndSubmitTasks)
    {
        if (!task->commandGraphs.empty())
        {
            ++numValidTasks;
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
        if (task->commandGraphs.size() == 1 && !task->transferTask)
        {
            // task only contains a single CommandGraph so keep thread simple
            auto run = [](ref_ptr<RecordAndSubmitTask> viewer_task, ref_ptr<FrameBlock> viewer_frameBlock, ref_ptr<Barrier> submissionCompleted, const std::string& threadName) {
                auto local_instrumentation = shareOrDuplicateForThreadSafety(viewer_task->instrumentation);
                if (local_instrumentation) local_instrumentation->setThreadName(threadName);

                auto frameStamp = viewer_frameBlock->initial_value;

                // wait for this frame to be signaled
                while (viewer_frameBlock->wait_for_change(frameStamp))
                {
                    CPU_INSTRUMENTATION_L1_NC(local_instrumentation, "Viewer run", COLOR_RECORD);

                    viewer_task->submit(frameStamp);

                    submissionCompleted->arrive_and_drop();
                }
            };

            threads.emplace_back(run, task, _frameBlock, _submissionCompleted, make_string("Viewer run thread"));
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
                    recordedCommandBuffers = RecordedCommandBuffers::create();
                    recordStartBarrier = Barrier::create(numThreads);
                    recordCompletedBarrier = Barrier::create(numThreads);
                }

                // shared between all threads
                ref_ptr<RecordAndSubmitTask> task;
                ref_ptr<FrameBlock> frameBlock;
                ref_ptr<Barrier> submissionCompletedBarrier;

                // shared between threads associated with each task
                ref_ptr<RecordedCommandBuffers> recordedCommandBuffers;
                ref_ptr<Barrier> recordStartBarrier;
                ref_ptr<Barrier> recordCompletedBarrier;
            };

            uint32_t numThreads = static_cast<uint32_t>(task->commandGraphs.size());
            if (task->transferTask) ++numThreads;

            ref_ptr<SharedData> sharedData = SharedData::create(task, _frameBlock, _submissionCompleted, numThreads);

            auto run_primary = [](ref_ptr<SharedData> data, ref_ptr<CommandGraph> commandGraph, const std::string& threadName) {
                auto local_instrumentation = shareOrDuplicateForThreadSafety(data->task->instrumentation);
                if (local_instrumentation) local_instrumentation->setThreadName(threadName);

                auto frameStamp = data->frameBlock->initial_value;

                // wait for this frame to be signaled
                while (data->frameBlock->wait_for_change(frameStamp))
                {
                    CPU_INSTRUMENTATION_L1_NC(local_instrumentation, "Viewer primary", COLOR_RECORD);

                    // primary thread starts the task
                    data->task->start();

                    data->recordStartBarrier->arrive_and_wait();

                    //vsg::info("run_primary");

                    commandGraph->record(data->recordedCommandBuffers, frameStamp, data->task->databasePager);

                    data->recordCompletedBarrier->arrive_and_wait();

                    // primary thread finishes the task, submitting all the command buffers recorded by the primary and all secondary threads to its queue
                    data->task->finish(data->recordedCommandBuffers);

                    data->recordedCommandBuffers->clear();

                    data->submissionCompletedBarrier->arrive_and_wait();
                }
            };

            auto run_secondary = [](ref_ptr<SharedData> data, ref_ptr<CommandGraph> commandGraph, const std::string& threadName) {
                auto local_instrumentation = shareOrDuplicateForThreadSafety(data->task->instrumentation);
                if (local_instrumentation) local_instrumentation->setThreadName(threadName);

                auto frameStamp = data->frameBlock->initial_value;

                // wait for this frame to be signaled
                while (data->frameBlock->wait_for_change(frameStamp))
                {
                    CPU_INSTRUMENTATION_L1_NC(local_instrumentation, "Viewer secondary", COLOR_RECORD);

                    data->recordStartBarrier->arrive_and_wait();

                    commandGraph->record(data->recordedCommandBuffers, frameStamp, data->task->databasePager);

                    data->recordCompletedBarrier->arrive_and_wait();
                }
            };

            auto run_transfer = [](ref_ptr<SharedData> data, ref_ptr<TransferTask> transferTask, TransferTask::TransferMask transferMask, const std::string& threadName) {
                auto local_instrumentation = shareOrDuplicateForThreadSafety(data->task->instrumentation);
                if (local_instrumentation) local_instrumentation->setThreadName(threadName);

                auto frameStamp = data->frameBlock->initial_value;

                // wait for this frame to be signaled
                while (data->frameBlock->wait_for_change(frameStamp))
                {
                    CPU_INSTRUMENTATION_L1_NC(local_instrumentation, "Viewer transfer", COLOR_RECORD);

                    data->recordStartBarrier->arrive_and_wait();

                    //vsg::info("run_transfer");

                    if (auto transfer = transferTask->transferData(transferMask); transfer.result == VK_SUCCESS)
                    {
                        if (transfer.dataTransferredSemaphore)
                        {
                            data->task->earlyDataTransferredSemaphore = transfer.dataTransferredSemaphore;
                        }
                    }

                    data->recordCompletedBarrier->arrive_and_wait();
                }
            };

            for (uint32_t i = 0; i < task->commandGraphs.size(); ++i)
            {
                if (i == 0)
                    threads.emplace_back(run_primary, sharedData, task->commandGraphs[i], make_string("Viewer primary thread"));
                else
                    threads.emplace_back(run_secondary, sharedData, task->commandGraphs[i], make_string("Viewer secondary thread ", i));
            }

            if (task->transferTask)
            {
                threads.emplace_back(run_transfer, sharedData, task->transferTask, TransferTask::TRANSFER_BEFORE_RECORD_TRAVERSAL, make_string("Viewer early transferTask thread"));
            }
        }
    }
}

void Viewer::stopThreading()
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer stopThreading", COLOR_VIEWER);

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
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer update", COLOR_UPDATE);

    // merge any updates from the DatabasePager
    for (const auto& task : recordAndSubmitTasks)
    {
        if (task->databasePager)
        {
            CompileResult cr;
            task->databasePager->updateSceneGraph(_frameStamp, cr);
            if (cr.requiresViewerUpdate()) updateViewer(*this, cr);
        }
    }

    // run update operations
    updateOperations->run();

    // run aniamtions
    animationManager->run(_frameStamp);
}

void Viewer::recordAndSubmit()
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer recordAndSubmitTask", COLOR_VIEWER);

    // reset connected ExecuteCommands
    for (const auto& recordAndSubmitTask : recordAndSubmitTasks)
    {
        for (auto& commandGraph : recordAndSubmitTask->commandGraphs)
        {
            commandGraph->reset();
        }
    }

#if 1
    if (_threading)
#else
    // The following is a workaround for an odd "Possible data race during write of size 1" warning that valgrind tool=helgrind reports
    // on the first call to vkBeginCommandBuffer despite them being done on independent command buffers.  This could well be a driver bug or a false positive.
    // If you want to quieten this warning then change the #if above to #if 0 as rendering the first three frames single threaded avoids the warning.
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
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "Viewer present", COLOR_VIEWER);

    for (auto& presentation : presentations)
    {
        presentation->present();
    }
}

void Viewer::assignInstrumentation(ref_ptr<Instrumentation> in_instrumentation)
{
    bool previous_threading = _threading;
    if (_threading) stopThreading();

    // don't change Instrumentation while devices are still active
    Viewer::deviceWaitIdle();

    instrumentation = in_instrumentation;

    // assign instrumentation after settings up recordAndSubmitTasks, but before compile() to allow compile to initialize the Instrumentation with the approach queue etc.
    for (auto& task : recordAndSubmitTasks)
    {
        task->assignInstrumentation(instrumentation);
    }

    if (compileManager) compileManager->assignInstrumentation(instrumentation);

    if (animationManager) animationManager->assignInstrumentation(instrumentation);

    if (previous_threading) setupThreading();
}

void vsg::updateViewer(Viewer& viewer, const CompileResult& compileResult)
{
    CPU_INSTRUMENTATION_L1_NC(viewer.instrumentation, "updateViewer", COLOR_VIEWER);

    updateTasks(viewer.recordAndSubmitTasks, viewer.compileManager, compileResult);
}
