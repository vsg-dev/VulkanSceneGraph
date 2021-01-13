#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/ui/WindowEvent.h>

namespace vsg
{
    class VSG_DECLSPEC ScrollWheelEvent : public Inherit<WindowEvent, ScrollWheelEvent>
    {
    public:
        ScrollWheelEvent() {}

        ScrollWheelEvent(Window* in_window, time_point in_time, const vec3& in_delta) :
            Inherit(in_window, in_time),
            delta(in_delta) {}

        /// scroll left delta.x < 0.0, scroll right delta.x > 0.0
        /// scroll up delta.y > 0.0, scroll down delta.y < 0.0
        /// scroll out delta.z > 0.0, scroll in delta.z < 0.0
        vec3 delta;

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::ScrollWheelEvent);

} // namespace vsg
