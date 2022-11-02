#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/Window.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/ui/WindowEvent.h>

namespace vsg
{

    /// TouchEvent is a base class touch events.
    class VSG_DECLSPEC TouchEvent : public Inherit<WindowEvent, TouchEvent>
    {
    public:
        TouchEvent() {}

        TouchEvent(Window* in_window, time_point in_time, uint32_t in_x, uint32_t in_y, uint32_t in_id) :
            Inherit(in_window, in_time),
            x(in_x),
            y(in_y),
            id(in_id) {}

        uint32_t x;
        uint32_t y;
        uint32_t id;

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::TouchEvent);

    /// TouchDownEvent represents a touch down event.
    class TouchDownEvent : public Inherit<TouchEvent, TouchDownEvent>
    {
    public:
        TouchDownEvent() {}

        TouchDownEvent(Window* in_window, time_point in_time, uint32_t in_x, uint32_t in_y, uint32_t in_id) :
            Inherit(in_window, in_time, in_x, in_y, in_id) {}
    };
    VSG_type_name(vsg::TouchDownEvent);

    /// TouchUpEvent represents a touch up event.
    class TouchUpEvent : public Inherit<TouchEvent, TouchUpEvent>
    {
    public:
        TouchUpEvent() {}

        TouchUpEvent(Window* in_window, time_point in_time, uint32_t in_x, uint32_t in_y, uint32_t in_id) :
            Inherit(in_window, in_time, in_x, in_y, in_id) {}
    };
    VSG_type_name(vsg::TouchUpEvent);

    /// TouchMoveEvent represents a touch move event.
    class TouchMoveEvent : public Inherit<TouchEvent, TouchMoveEvent>
    {
    public:
        TouchMoveEvent() {}

        TouchMoveEvent(Window* in_window, time_point in_time, uint32_t in_x, uint32_t in_y, uint32_t in_id) :
            Inherit(in_window, in_time, in_x, in_y, in_id) {}
    };
    VSG_type_name(vsg::TouchMoveEvent);

} // namespace vsg
