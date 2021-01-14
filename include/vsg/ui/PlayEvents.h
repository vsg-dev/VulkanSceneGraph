#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>

namespace vsg
{

    class VSG_DECLSPEC PlayEvents : public vsg::Inherit<vsg::Visitor, PlayEvents>
    {
    public:
        PlayEvents(vsg::ref_ptr<vsg::Object> object, vsg::clock::time_point::duration delta);

        bool resetHandled = true;

        using Events = std::list<vsg::ref_ptr<vsg::UIEvent>>;
        Events events;
        Events::iterator events_itr;
        bool frameEnd = false;

        // cache of events to dispatch for current frame.
        Events frameEvents;

        bool dispatchFrameEvents(vsg::UIEvents& viewer_events);

        void apply(vsg::UIEvent& event) override;

        void apply(vsg::FrameEvent& event) override;

        void apply(vsg::ConfigureWindowEvent& event) override;

        void apply(vsg::ExposeWindowEvent& event) override;
    };

} // namespace vsg
