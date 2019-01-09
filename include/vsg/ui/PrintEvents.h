#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>

#include <iostream>

namespace vsg
{
    struct PrintEvents : public vsg::Visitor
    {
        vsg::clock::time_point start_point;

        PrintEvents(vsg::clock::time_point in_start_point) :
            start_point(in_start_point) {}

        void apply(vsg::UIEvent& event)
        {
            std::cout << "event : " << event.className() << ", " << std::chrono::duration<double>(event.time - start_point).count() << std::endl;
        }

        void apply(vsg::ExposeWindowEvent& event)
        {
            std::cout << "Expose window : " << event.className() << ", " << std::chrono::duration<double>(event.time - start_point).count() << " " << event.x << ", " << event.y << ", " << event.width << ", " << event.height << std::endl;
        }

        void apply(vsg::CloseWindowEvent& event)
        {
            std::cout << "Close window : " << event.className() << ", " << std::chrono::duration<double>(event.time - start_point).count() << std::endl;
        }

        void apply(vsg::KeyReleaseEvent& keyRelease)
        {
            std::cout << "KeyReleaeEvent : " << keyRelease.className() << ", " << std::chrono::duration<double>(keyRelease.time - start_point).count() << ", " << keyRelease.keyBase << std::endl;
        }

        void apply(vsg::KeyPressEvent& keyPress)
        {
            std::cout << "KeyPressEvent : " << keyPress.className() << ", " << std::chrono::duration<double>(keyPress.time - start_point).count() << ", " << keyPress.keyBase << ", " << keyPress.keyModified << std::endl;
        }

        void apply(vsg::ButtonPressEvent& buttonPress)
        {
            std::cout << "ButtonPress : " << buttonPress.className() << ", " << std::chrono::duration<double>(buttonPress.time - start_point).count() << ", " << buttonPress.x << ", " << buttonPress.y << ", " << buttonPress.mask << ", " << buttonPress.button << std::endl;
        }

        void apply(vsg::ButtonReleaseEvent& buttonRelease)
        {
            std::cout << "ButtonRelease : " << buttonRelease.className() << ", " << std::chrono::duration<double>(buttonRelease.time - start_point).count() << ", " << buttonRelease.x << ", " << buttonRelease.y << ", " << buttonRelease.mask << ", " << buttonRelease.button << std::endl;
        }

        void apply(vsg::MoveEvent& move)
        {
            std::cout << "MoveEvent : " << move.className() << ", " << std::chrono::duration<double>(move.time - start_point).count() << ", " << move.x << ", " << move.y << ", " << move.mask << std::endl;
        }
    };
}
