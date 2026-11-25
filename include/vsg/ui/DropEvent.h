#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2025 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Path.h>
#include <vsg/ui/WindowEvent.h>

namespace vsg
{

    /// DropEvent is a base class for the drag and drop of files from the desktop onto a Window.
    ///
    /// The window's x, y position is in window coordinates, as for PointerEvent. paths holds the
    /// files being dragged; note that under X11 the names are only transferred once the button is
    /// released, so paths is empty on the hover events there and only DropFilesEvent is complete.
    class VSG_DECLSPEC DropEvent : public Inherit<WindowEvent, DropEvent>
    {
    public:
        DropEvent() {}

        DropEvent(Window* in_window, time_point in_time, int32_t in_x, int32_t in_y, const Paths& in_paths = {}) :
            Inherit(in_window, in_time),
            x(in_x),
            y(in_y),
            paths(in_paths) {}

        int32_t x = 0;
        int32_t y = 0;
        Paths paths;

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::DropEvent);

    /// DropHoverEvent is sent each frame while a drag is held over the window.
    ///
    /// Handlers set accept to say whether a drop at this position would be taken, which the window
    /// passes back to the source so it can show the right cursor. accept starts false, so a window
    /// with no interest in drops rejects them by doing nothing.
    class VSG_DECLSPEC DropHoverEvent : public Inherit<DropEvent, DropHoverEvent>
    {
    public:
        DropHoverEvent() {}

        DropHoverEvent(Window* in_window, time_point in_time, int32_t in_x, int32_t in_y, const Paths& in_paths = {}) :
            Inherit(in_window, in_time, in_x, in_y, in_paths) {}

        bool accept = false;

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::DropHoverEvent);

    /// DropLeaveEvent is sent when the drag leaves the window or is cancelled, and after a drop has
    /// been delivered, so a handler has a single place to clear any hover state.
    class VSG_DECLSPEC DropLeaveEvent : public Inherit<DropEvent, DropLeaveEvent>
    {
    public:
        DropLeaveEvent() {}

        DropLeaveEvent(Window* in_window, time_point in_time, int32_t in_x, int32_t in_y) :
            Inherit(in_window, in_time, in_x, in_y) {}
    };
    VSG_type_name(vsg::DropLeaveEvent);

    /// DropFilesEvent is sent when the files have been dropped and their names are known.
    class VSG_DECLSPEC DropFilesEvent : public Inherit<DropEvent, DropFilesEvent>
    {
    public:
        DropFilesEvent() {}

        DropFilesEvent(Window* in_window, time_point in_time, int32_t in_x, int32_t in_y, const Paths& in_paths) :
            Inherit(in_window, in_time, in_x, in_y, in_paths) {}
    };
    VSG_type_name(vsg::DropFilesEvent);

} // namespace vsg
