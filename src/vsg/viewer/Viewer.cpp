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
    _status = vsg::ActivityStatus::create();
}

Viewer::~Viewer()
{
    // don't kill window while devices are still _status
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

void Viewer::close()
{
    _close = true;
    _status->set(false);
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
        auto maxSets = collectStats.computeNumDescriptorSets();
        const auto& descriptorPoolSizes = collectStats.computeDescriptorPoolSizes();

        auto queueFamily = physicalDevice->getQueueFamily(VK_QUEUE_GRAPHICS_BIT); // TODO : could we just use transfer bit?

        deviceResource.compile = new vsg::CompileTraversal(device, bufferPreferences);
        deviceResource.compile->context.commandPool = vsg::CommandPool::create(device, queueFamily);
        deviceResource.compile->context.graphicsQueue = device->getQueue(queueFamily);

        if (descriptorPoolSizes.size() > 0) deviceResource.compile->context.descriptorPool = vsg::DescriptorPool::create(device, maxSets, descriptorPoolSizes);
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

void Viewer::assignRecordAndSubmitTaskAndPresentation(CommandGraphs in_commandGraphs, DatabasePager* databasePager)
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

    // place the input CommandGraphs into seperate groups associated with each device and queue family combination
    std::map<DeviceQueueFamily, CommandGraphs> deviceCommandGraphsMap;
    for (auto& commandGraph : in_commandGraphs)
    {
        deviceCommandGraphsMap[DeviceQueueFamily{commandGraph->_device.get(), commandGraph->_queueFamily, commandGraph->_presentFamily}].emplace_back(commandGraph);
    }

    // create the required RecordAndSubmitTask and any Presentation objecst that are required for each set of CommandGraphs
    for (auto& [deviceQueueFamily, commandGraphs] : deviceCommandGraphsMap)
    {
        auto device = deviceQueueFamily.device;
        if (deviceQueueFamily.presentFamily >= 0)
        {
            // collate all the unique Windows associaged with these commandGraphs
            std::set<Window*> uniqueWindows;
            for (auto& commanGraph : commandGraphs)
            {
                uniqueWindows.insert(commanGraph->window);
            }

            Windows windows(uniqueWindows.begin(), uniqueWindows.end());

            auto renderFinishedSemaphore = vsg::Semaphore::create(device);

            // set up Submission with CommandBuffer and signals
            auto recordAndSubmitTask = vsg::RecordAndSubmitTask::create();
            recordAndSubmitTask->commandGraphs = commandGraphs;
            recordAndSubmitTask->signalSemaphores.emplace_back(renderFinishedSemaphore);
            recordAndSubmitTask->databasePager = databasePager;
            recordAndSubmitTask->windows = windows;
            recordAndSubmitTask->queue = device->getQueue(deviceQueueFamily.queueFamily);
            recordAndSubmitTasks.emplace_back(recordAndSubmitTask);

            auto presentation = vsg::Presentation::create();
            presentation->waitSemaphores.emplace_back(renderFinishedSemaphore);
            presentation->windows = windows;
            presentation->queue = device->getQueue(deviceQueueFamily.presentFamily);
            presentations.emplace_back(presentation);
        }
        else
        {
            // with don't have a presentFamily so this set of commandGraphs aren't associated with a widnow
            // set up Submission with CommandBuffer and signals
            auto recordAndSubmitTask = vsg::RecordAndSubmitTask::create();
            recordAndSubmitTask->commandGraphs = commandGraphs;
            recordAndSubmitTask->databasePager = databasePager;
            recordAndSubmitTask->queue = device->getQueue(deviceQueueFamily.queueFamily);
            recordAndSubmitTasks.emplace_back(recordAndSubmitTask);
        }
    }
}

class FrameBlock : public Inherit<Object, FrameBlock>
{
public:

    inline static const ref_ptr<FrameStamp> initial_value = {};

    FrameBlock(ref_ptr<ActivityStatus> status) : _value(initial_value), _status(status) {}

    FrameBlock(const FrameBlock&) = delete;
    FrameBlock& operator = (const FrameBlock&) = delete;

    void set(ref_ptr<FrameStamp> frameStamp)
    {
        std::scoped_lock lock(_mutex);
        _value = frameStamp;
        _cv.notify_all();
    }

    ref_ptr<FrameStamp> get()
    {
        std::scoped_lock lock(_mutex);
        return _value;
    }

    bool active() const { return _status->active(); }

    void wake()
    {
        std::scoped_lock lock(_mutex);
        _cv.notify_all();
    }

    bool wait_for_change(ref_ptr<FrameStamp>& value)
    {
        std::unique_lock lock(_mutex);
        while (_value == value && _status->active())
        {
            _cv.wait(lock);
        }

        value = _value;
        return _status->active();
    }

protected:
    virtual ~FrameBlock() {}

    std::mutex _mutex;
    std::condition_variable _cv;
    ref_ptr<FrameStamp> _value;
    ref_ptr<ActivityStatus> _status;
};


class Barrier : public Inherit<Object, Barrier>
{
public:
    Barrier(int num) :
        _num_threads(num) {}

    Barrier(const Barrier&) = delete;
    Barrier& operator = (const Barrier&) = delete;

    void reset()
    {
        _count = _num_threads;
    }

    void arrive_and_wait()
    {
        if (_count.fetch_sub(1) <= 1)
        {
            std::scoped_lock lock(_mutex);
            _cv.notify_all();

            released();
        }
        else
        {
            wait();
        }
    }

    void arrive_and_drop()
    {
        if (_count.fetch_sub(1) <= 1)
        {
            std::scoped_lock lock(_mutex);
            _cv.notify_all();

            released();
        }
    }

    void wait()
    {
        std::unique_lock lock(_mutex);
        while (_count > 0)
        {
            _cv.wait(lock);
        }
    }

    bool is_ready() const
    {
        return _count==0;
    }

    virtual void released() {}

protected:
    virtual ~Barrier() {}

    const int _num_threads;
    std::atomic_int _count;
    std::mutex _mutex;
    std::condition_variable _cv;
};

struct SubmitBarrier : public Inherit<Barrier, SubmitBarrier>
{
    SubmitBarrier(int num) : Inherit(num) {}
    void submit(const CommandBuffers& rcb)
    {
        {
            std::scoped_lock lock(recordCommandBuffersMutex);
            recordedCommandBuffers.insert(recordedCommandBuffers.end(), rcb.begin(), rcb.end());
        }

        arrive_and_drop();
    }

    void released() override
    {
        std::scoped_lock lock(recordCommandBuffersMutex);

        // do submissions

        submissionCompleteBarrier->arrive_and_drop();
    }

    std::mutex recordCommandBuffersMutex;
    CommandBuffers recordedCommandBuffers;
    ref_ptr<Barrier> submissionCompleteBarrier;
};


#include <vsg/threading/Affinity.h>

void Viewer::setupThreading()
{
    ref_ptr<FrameBlock> frameBlock = FrameBlock::create(_status);

    std::vector<ref_ptr<SubmitBarrier>> submitBarriers;

    ref_ptr<Barrier> submissionsCompleteBarrier = Barrier::create(recordAndSubmitTasks.size());

    uint32_t cpu_num = 2;
    uint32_t cpu_increment = 2;

    for(auto& task : recordAndSubmitTasks)
    {
        ref_ptr<SubmitBarrier> submitBarrier = SubmitBarrier::create(task->commandGraphs.size());
        submitBarriers.emplace_back(submitBarrier);

        submitBarrier->submissionCompleteBarrier = submissionsCompleteBarrier;

        for(auto& commandGraph : task->commandGraphs)
        {
            auto run = [](observer_ptr<CommandGraph> cg, ref_ptr<FrameBlock> viewer_frameBlock, ref_ptr<SubmitBarrier> task_submitBarrier, ref_ptr<DatabasePager> /*databasePager*/)
            {
                auto frameStamp = viewer_frameBlock->initial_value;

                // wait for this frame to be signalled
                while(viewer_frameBlock->wait_for_change(frameStamp))
                {
                    // take a refernce to the command buffer to prevent it being deleted while we are traversing.
                    ref_ptr<CommandGraph> rcg = cg;
                    if (!rcg) break;

                    // record the command buffer
                    CommandBuffers recordedCommandBuffers;
#if 0
                    rcg->record(recordedCommandBuffers, frameStamp, databasePager);

#else
                    auto count = frameStamp->frameCount;
                    for(int i=0; i<1000009; ++i) count = count*2 -3;
#endif

                    //std::cout<<"run()  "<<rcg.get()<<", frameCount = "<<std::dec<<frameStamp->frameCount<<" "<<databasePager.get()<<std::endl;

                    // pass the result of this record traversal onto the submitBarrier
                    task_submitBarrier->submit(recordedCommandBuffers);

                }

                std::cout<<"Exiting thread"<<std::endl;
            };



            // run(observer_ptr<CommandGraph>(commandGraph), frameBlock, submitBarrier, task->databasePager, _status);

//            commandGraph->thread = std::thread(run, observer_ptr<CommandGraph>(commandGraph), std::ref(frameBlock), std::ref(submitBarrier), std::ref(task->databasePager));
            commandGraph->thread = std::thread(run, observer_ptr<CommandGraph>(commandGraph), frameBlock, submitBarrier, task->databasePager);

            if (cpu_increment > 0)
            {
                setAffinity(commandGraph->thread, cpu_num);
                cpu_num += cpu_increment;
            }
        }
    }

    for(uint64_t i = 0; i<100000; ++i)
    {
        submissionsCompleteBarrier->reset();
        frameBlock->set(FrameStamp::create(vsg::clock::now(), i));
        submissionsCompleteBarrier->wait();
    }

    // release the blocks to enable threads to exit cleanly
    // need to manually wake up the threads waiting on this frameBlock so they check the _status value and exit cleanly.
    _status->set(false);
    frameBlock->wake();

    for(auto& task : recordAndSubmitTasks)
    {
        for(auto& commandGraph : task->commandGraphs)
        {
            commandGraph->thread.join();
        }
    }
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
    // TODO:Multi-threading approach notes:
    //
    //   CommandGraph "has a" std::thread  (CG_Thread)
    //   CommandGraph "has a" vsg::Affinity? Used when configuring the CG_Thread
    //
    //   CG_Thread "has a" RecordTraversalStartBarrier (vsg::Latch?) could be shared?
    //   CG Thread "has a RecordTraverasl to record command graph
    //   CG_Thread "has a shared" ReordTraversalFinishiedBarrier
    //
    //   RecordTraveraslBarrier is joined by N CG_Thread's and passed the commandbuffers to submit
    //   One per RecordAndSubmitTask
    //   Once all the associated CG_Threads have joined the RecordTraveraslBarrier it's released and the VkSubmit occurs
    //   Then the RecordTraveraslBarrier joins a SubmitFinsihedBarrier
    //
    //   SubmitFinsihedBarrier is joined by M RecordTraveraslBarrier's after they have submitted there work
    //   Main thread can wait on SubmitFinsihedBarrier within recordAndSubmit or optionally by the application in it's main loop,
    //   or at latest at the start of the recordAndSubmit().
    //
    //
    //      Need to create a set of multi-threading test cases to develop for:
    //          Multi-gpu
    //          Multi-pass
    //          Mulit-window/viewport
    //          Compute and Graphics
    //          All of the above with DatabasePaging

    for (auto& recordAndSubmitTask : recordAndSubmitTasks)
    {
        recordAndSubmitTask->submit(_frameStamp);
    }
}

void Viewer::present()
{
    for (auto& presentation : presentations)
    {
        presentation->present();
    }
}
