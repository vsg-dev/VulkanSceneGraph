#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/observer_ptr.h>
#include <vsg/ui/WindowEvent.h>
#include <vsg/viewer/Window.h>

namespace vsg
{

    enum ButtonMask : uint16_t
    {
        BUTTON_MASK_1 = 256,
        BUTTON_MASK_2 = 512,
        BUTTON_MASK_3 = 1024,
        BUTTON_MASK_4 = 2048, /// mouse wheel up
        BUTTON_MASK_5 = 4096  /// mouse wheel down
    };

    VSG_type_name(vsg::PointerEvent);
    class VSG_DECLSPEC PointerEvent : public Inherit<WindowEvent, PointerEvent>
    {
    public:
        PointerEvent() {}

        PointerEvent(Window* in_window, time_point in_time, int32_t in_x, int32_t in_y, ButtonMask in_buttonMask) :
            Inherit(in_window, in_time),
            x(in_x),
            y(in_y),
            mask(in_buttonMask) {}

        int32_t x = 0;
        int32_t y = 0;
        ButtonMask mask = {};

        void read(Input& input) override;
        void write(Output& output) const override;
    };

    VSG_type_name(vsg::ButtonPressEvent);
    class VSG_DECLSPEC ButtonPressEvent : public Inherit<PointerEvent, ButtonPressEvent>
    {
    public:
        ButtonPressEvent() {}

        ButtonPressEvent(Window* in_window, time_point in_time, int32_t in_x, int32_t in_y, ButtonMask in_buttonMask, uint32_t in_button) :
            Inherit(in_window, in_time, in_x, in_y, in_buttonMask),
            button(in_button) {}

        uint32_t button = 0;

        void read(Input& input) override;
        void write(Output& output) const override;
    };

    VSG_type_name(vsg::ButtonReleaseEvent);
    class VSG_DECLSPEC ButtonReleaseEvent : public Inherit<PointerEvent, ButtonReleaseEvent>
    {
    public:
        ButtonReleaseEvent() {}

        ButtonReleaseEvent(Window* in_window, time_point in_time, int32_t in_x, int32_t in_y, ButtonMask in_buttonMask, uint32_t in_button) :
            Inherit(in_window, in_time, in_x, in_y, in_buttonMask),
            button(in_button) {}

        uint32_t button;

        void read(Input& input) override;
        void write(Output& output) const override;
    };

    VSG_type_name(vsg::MoveEvent);
    class MoveEvent : public Inherit<PointerEvent, MoveEvent>
    {
    public:
        MoveEvent() {}

        MoveEvent(Window* in_window, time_point in_time, int32_t in_x, int32_t in_y, ButtonMask in_buttonMask) :
            Inherit(in_window, in_time, in_x, in_y, in_buttonMask) {}
    };

} // namespace vsg
