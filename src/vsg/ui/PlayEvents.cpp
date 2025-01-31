/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/ui/CollectEvents.h>
#include <vsg/ui/PlayEvents.h>
#include <vsg/ui/ShiftEventTime.h>

using namespace vsg;

PlayEvents::PlayEvents(vsg::ref_ptr<vsg::Object> object, vsg::clock::time_point::duration delta)
{
    ShiftEventTime shiftTime(delta);
    object->accept(shiftTime);

    CollectEvents collectEvents;
    object->accept(collectEvents);

    events = collectEvents.events;
    events_itr = events.begin();
    frameEnd = false;
}

bool PlayEvents::dispatchFrameEvents(vsg::UIEvents& viewer_events)
{
    // find the last (w.r.t time) event in the viewer_events - typically the viewer's FrameEvent
    auto max_itr = std::max_element(viewer_events.begin(), viewer_events.end(), [](const vsg::ref_ptr<vsg::UIEvent>& lhs, const vsg::ref_ptr<vsg::UIEvent>& rhs) {
        return lhs->time < rhs->time;
    });

    // clear the list of frameEvents that we will want to pass to the viewer
    // to be filled in by the PlayUIEvents::apply(..) methods.
    frameEnd = false;
    frameEvents.clear();

    if (max_itr != viewer_events.end())
    {
        // handle all the PlayUIEvents::events before the final event in the viewer_events.
        while (events_itr != events.end() && (*events_itr)->time < (*max_itr)->time)
        {
            (*events_itr)->accept(*this);
            ++events_itr;
        }
    }
    else
    {
        // fallback to use FrameEvent in the PlayUIEvents::events as boundary for events to dispatch
        while (events_itr != events.end())
        {
            (*events_itr)->accept(*this);
            ++events_itr;

            if (frameEnd) break;
        }
    }

    viewer_events.insert(viewer_events.begin(), frameEvents.begin(), frameEvents.end());

    return events_itr != events.end();
}

void PlayEvents::apply(vsg::UIEvent& event)
{
    if (resetHandled) event.handled = false;

    frameEvents.push_back(vsg::ref_ptr<vsg::UIEvent>(&event));
}

void PlayEvents::apply(vsg::FrameEvent& /*event*/)
{
    frameEnd = true;
}

void PlayEvents::apply(vsg::ConfigureWindowEvent& /*event*/)
{
    // ignore, long term should we be sending events to the Window?
}

void PlayEvents::apply(vsg::ExposeWindowEvent& /*event*/)
{
    // ignore for now, long term should we be sending events to the Window?
}
