#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Visitor.h>
#include <vsg/ui/UIEvent.h>

namespace vsg
{
    class VSG_DECLSPEC PrintEvents : public Inherit<Visitor, PrintEvents>
    {
    public:
        explicit PrintEvents(clock::time_point in_start_point);
        PrintEvents(std::ostream& out, clock::time_point in_start_point);

        std::ostream& output;
        clock::time_point start_point;

        virtual std::ostream& print(UIEvent& event);

        void apply(Object& object) override;
        void apply(UIEvent& event) override;
        void apply(FrameEvent& event) override;
        void apply(ExposeWindowEvent& event) override;
        void apply(CloseWindowEvent& event) override;
        void apply(KeyReleaseEvent& keyRelease) override;
        void apply(KeyPressEvent& keyPress) override;
        void apply(ButtonPressEvent& buttonPress) override;
        void apply(ButtonReleaseEvent& buttonRelease) override;
        void apply(MoveEvent& move) override;
        void apply(ScrollWheelEvent& scrollWheel) override;
    };
} // namespace vsg
