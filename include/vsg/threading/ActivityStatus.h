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

    /// Atomic management of whether threads watching this ActivityStatus object should be active or safely exit
    class ActivityStatus : public Inherit<Object, ActivityStatus>
    {
    public:
        ActivityStatus(bool active = true) :
            _active(active) {}

        void set(bool flag) noexcept { _active.exchange(flag); }

        /// return true if the caller should continue with current activity or false if they should be cancelled
        bool active() const noexcept { return _active; }

        /// return true if the caller should cancel current activity
        bool cancel() const noexcept { return !_active; }

    protected:
        virtual ~ActivityStatus() {}

        std::atomic_bool _active;
    };
    VSG_type_name(vsg::ActivityStatus)

} // namespace vsg
