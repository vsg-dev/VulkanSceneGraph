#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/threading/ActivityStatus.h>
#include <vsg/ui/ApplicationEvent.h>

namespace vsg
{

    class FrameBlock : public Inherit<Object, FrameBlock>
    {
    public:
        inline static const ref_ptr<FrameStamp> initial_value = {};

        explicit FrameBlock(ref_ptr<ActivityStatus> status) :
            _value(initial_value),
            _status(status) {}

        FrameBlock(const FrameBlock&) = delete;
        FrameBlock& operator=(const FrameBlock&) = delete;

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
    VSG_type_name(vsg::FrameBlock);

} // namespace vsg
