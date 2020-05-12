#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>

namespace vsg
{

    class Barrier : public Inherit<Object, Barrier>
    {
    public:
        Barrier(int num) :
            _num_threads(num),
            _count(num) {}

        Barrier(const Barrier&) = delete;
        Barrier& operator=(const Barrier&) = delete;

        void reset()
        {
            _count = _num_threads;
        }

        /// decrement the Barrier count, if count goes to zero call Barier::release() to release all waiting threads, otherwsie wait on the barrier.
        void arrive_and_wait()
        {
            if (_count.fetch_sub(1) <= 1)
            {
                release();
            }
            else
            {
                wait();
            }
        }

        /// decrement the Barrier count, if thread is one to reduce the count to return immediately and return true, otherwise wait on barrier and when it's released return false.
        /// If true is reqturn it is the callers responsibility to call Battier::release() to release all waiting thrads.
        bool arrive_and_wait_or_manual_release()
        {
            if (_count.fetch_sub(1) <= 1)
            {
                return true;
            }
            else
            {
                wait();
                return false;
            }
        }

        /// decrement the Barrier count and return immediately, and if it goes to zero calls Barrier::release() to release all waiting threads
        void arrive_and_drop()
        {
            if (_count.fetch_sub(1) <= 1)
            {
                release();
            }
        }

        /// wait on barrier till it's count goes to zero.
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
            return _count == 0;
        }

        /// release all waiting threads.
        virtual void release()
        {
            std::scoped_lock lock(_mutex);
            _cv.notify_all();
        }

    protected:
        virtual ~Barrier() {}

        const int _num_threads;
        std::atomic_int _count;
        std::mutex _mutex;
        std::condition_variable _cv;
    };
    VSG_type_name(vsg::Barrier);

} // namespace vsg
