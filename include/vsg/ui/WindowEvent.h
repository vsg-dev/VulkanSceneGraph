#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/observer_ptr.h>
#include <vsg/ui/UIEvent.h>
#include <vsg/viewer/Window.h>

namespace vsg
{

    VSG_type_name(vsg::WindowEvent);
    class VSG_DECLSPEC WindowEvent : public Inherit<UIEvent, WindowEvent>
    {
    public:
        WindowEvent() {}

        WindowEvent(Window* in_window, time_point in_time) :
            Inherit(in_time),
            window(in_window) {}

        observer_ptr<Window> window;

        void read(Input& input) override;
        void write(Output& output) const override;
    };

    VSG_type_name(vsg::ExposeWindowEvent);
    class VSG_DECLSPEC ExposeWindowEvent : public Inherit<WindowEvent, ExposeWindowEvent>
    {
    public:
        ExposeWindowEvent() {}

        ExposeWindowEvent(Window* in_window, time_point in_time, int32_t in_x, int32_t in_y, uint32_t in_width, uint32_t in_height) :
            Inherit(in_window, in_time),
            x(in_x),
            y(in_y),
            width(in_width),
            height(in_height) {}

        int32_t x = 0;
        int32_t y = 0;
        uint32_t width = 0;
        uint32_t height = 0;

        void read(Input& input) override;
        void write(Output& output) const override;
    };

    VSG_type_name(vsg::ConfigureWindowEvent);
    class VSG_DECLSPEC ConfigureWindowEvent : public Inherit<WindowEvent, ConfigureWindowEvent>
    {
    public:
        ConfigureWindowEvent() {}

        ConfigureWindowEvent(Window* in_window, time_point in_time, int32_t in_x, int32_t in_y, uint32_t in_width, uint32_t in_height) :
            Inherit(in_window, in_time),
            x(in_x),
            y(in_y),
            width(in_width),
            height(in_height) {}

        int32_t x = 0;
        int32_t y = 0;
        uint32_t width = 0;
        uint32_t height = 0;

        void read(Input& input) override;
        void write(Output& output) const override;
    };

    VSG_type_name(vsg::CloseWindowEvent);
    class CloseWindowEvent : public Inherit<WindowEvent, CloseWindowEvent>
    {
    public:
        CloseWindowEvent() {}

        CloseWindowEvent(Window* in_window, time_point in_time) :
            Inherit(in_window, in_time) {}
    };

} // namespace vsg
