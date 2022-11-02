#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>

#include <condition_variable>
#include <mutex>

namespace vsg
{

    /// Latch provides a means for synchronization multiple threads that waits for the latch count to be decremented to zero.
    class Latch : public Inherit<Object, Latch>
    {
    public:
        explicit Latch(int num) :
            _count(num) {}

        explicit Latch(size_t num) :
            Latch(static_cast<int>(num)) {}

        void set(int num)
        {
            if (num == 0)
            {
                if (_count.exchange(0) != 0)
                    release();
            }
            else
            {
                _count = num;
            }
        }

        void count_up()
        {
            ++_count;
        }

        bool count_down()
        {
            if (_count.fetch_sub(1) <= 1)
            {
                release();
                return true;
            }
            else
            {
                return false;
            }
        }

        bool is_ready() const
        {
            return (_count <= 0);
        }

        void wait()
        {
            std::unique_lock lock(_mutex);
            while (_count > 0)
            {
                _cv.wait(lock);
            }
        }

        virtual void release()
        {
            std::unique_lock lock(_mutex);
            _cv.notify_all();
        }

        int count() const { return _count.load(); }

    protected:
        virtual ~Latch() {}

        std::atomic_int _count;
        std::mutex _mutex;
        std::condition_variable _cv;
    };
    VSG_type_name(vsg::Latch)

} // namespace vsg
