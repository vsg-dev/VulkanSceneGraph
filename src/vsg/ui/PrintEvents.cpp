/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/stream.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/PrintEvents.h>
#include <vsg/ui/ScrollWheelEvent.h>

#include <iostream>
#include <sstream>

using namespace vsg;

PrintEvents::PrintEvents(clock::time_point in_start_point) :
    output(std::cout),
    start_point(in_start_point)
{
}

PrintEvents::PrintEvents(std::ostream& out, clock::time_point in_start_point) :
    output(out),
    start_point(in_start_point)
{
}

std::ostream& PrintEvents::print(UIEvent& event)
{
    output << event.className() << ", " << std::chrono::duration<double, std::chrono::milliseconds::period>(event.time - start_point).count() << "ms";
    return output;
}

void PrintEvents::apply(Object& object)
{
    object.traverse(*this);
}

void PrintEvents::apply(UIEvent& event)
{
    print(event) << std::endl;
}

void PrintEvents::apply(FrameEvent& event)
{
    print(event) << " : frameCount = " << event.frameStamp->frameCount << std::endl;
}

void PrintEvents::apply(ExposeWindowEvent& event)
{
    print(event) << " : x = " << event.x << ", y = " << event.y << ", width = " << event.width << ", height = " << event.height << std::endl;
}

void PrintEvents::apply(CloseWindowEvent& event)
{
    print(event) << std::endl;
}

void PrintEvents::apply(KeyReleaseEvent& keyRelease)
{
    print(keyRelease) << " : keyBase = " << keyRelease.keyBase << ", keyModified = " << keyRelease.keyModified << std::endl;
}

void PrintEvents::apply(KeyPressEvent& keyPress)
{
    print(keyPress) << " : keyBase = " << keyPress.keyBase << ", keyModified = " << keyPress.keyModified << std::endl;
}

void PrintEvents::apply(ButtonPressEvent& buttonPress)
{
    print(buttonPress) << " : x = " << buttonPress.x << ", y = " << buttonPress.y << ", mask =" << buttonPress.mask << ", button =" << buttonPress.button << std::endl;
}

void PrintEvents::apply(ButtonReleaseEvent& buttonRelease)
{
    print(buttonRelease) << " : x = " << buttonRelease.x << ", y = " << buttonRelease.y << ", mask = " << buttonRelease.mask << ", button = " << buttonRelease.button << std::endl;
}

void PrintEvents::apply(MoveEvent& move)
{
    print(move) << " : x = " << move.x << ", y =" << move.y << ", mask = " << move.mask << std::endl;
}

void PrintEvents::apply(ScrollWheelEvent& scrollWheel)
{
    print(scrollWheel) << " : delta " << scrollWheel.delta << std::endl;
}
