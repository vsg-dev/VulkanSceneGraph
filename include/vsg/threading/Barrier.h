#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>

#include <condition_variable>

namespace vsg
{

    /// Barrier provides a means for synchronization multiple threads that all release together once specified number of threads joined the Barrier.
    class Barrier : public Inherit<Object, Barrier>
    {
    public:
        explicit Barrier(uint32_t num_thread) :
            _num_threads(num_thread),
            _num_arrived(0),
            _phase(0) {}

        Barrier(const Barrier&) = delete;
        Barrier& operator=(const Barrier&) = delete;

        /// increment the arrived count and release the barrier if count matches number of threads to arrive otherwise waiting for the arrived count to match the number if threads to arrive
        void arrive_and_wait()
        {
            std::unique_lock lock(_mutex);
            if (++_num_arrived == _num_threads)
            {
                _release();
            }
            else
            {
                auto my_phase = _phase;
                _cv.wait(lock, [this, my_phase]() { return this->_phase != my_phase; });
            }
        }

        /// increment the arrived count and release the barrier if count matches number of threads to arrive, return immediately without waiting for release condition
        void arrive_and_drop()
        {
            std::unique_lock lock(_mutex);
            if (++_num_arrived == _num_threads)
            {
                _release();
            }
        }

    protected:
        virtual ~Barrier() {}

        void _release()
        {
            _num_arrived = 0;
            ++_phase;
            _cv.notify_all();
        }

        const uint32_t _num_threads;
        uint32_t _num_arrived;
        uint32_t _phase;

        std::mutex _mutex;
        std::condition_variable _cv;
    };
    VSG_type_name(vsg::Barrier);

} // namespace vsg
