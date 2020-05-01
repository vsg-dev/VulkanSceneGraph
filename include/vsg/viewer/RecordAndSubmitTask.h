#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Group.h>

#include <vsg/vk/CommandBuffer.h>

#include <vsg/io/DatabasePager.h>

#include <vsg/viewer/CommandGraph.h>
#include <vsg/viewer/Window.h>

namespace vsg
{

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

    // RecordAndSubmitTask
    class RecordAndSubmitTask : public Inherit<Object, RecordAndSubmitTask>
    {
    public:
        RecordAndSubmitTask(Device* device, uint32_t numBuffers = 3);

        // Need to add FrameStamp?
        virtual VkResult submit(ref_ptr<FrameStamp> frameStamp = {});

        virtual VkResult start();
        virtual VkResult record(CommandBuffers& recordedCommandBuffers, ref_ptr<FrameStamp> frameStamp);
        virtual VkResult finish(CommandBuffers& recordedCommandBuffers);

        Windows windows;
        Semaphores waitSemaphores;
        CommandGraphs commandGraphs; // assign in application setup
        Semaphores signalSemaphores; // connect to Presentation.waitSemaphores

        uint32_t index = 0; // current fence/buffer set to use, cycled through automatically in submit call.
        std::vector<ref_ptr<Fence>> fences;

        ref_ptr<Queue> queue; // assign in application for GraphicsQueue from device

        ref_ptr<DatabasePager> databasePager;

        std::thread thread;
    };

} // namespace vsg
