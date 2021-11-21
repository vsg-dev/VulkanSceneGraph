#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/ui/KeyEvent.h>
#include <vsg/viewer/Viewer.h>

namespace vsg
{

    class CloseHandler : public Inherit<Visitor, CloseHandler>
    {
    public:
        CloseHandler(Viewer* viewer) :
            _viewer(viewer) {}

        KeySymbol closeKey = KeySymbol::KEY_Escape; // KEY_Undefined

        virtual void close()
        {
            // take a ref_ptr<> of the oberserv_ptr<> to be able to safely access it
            ref_ptr<Viewer> viewer = _viewer;
            if (viewer) viewer->close();
        }

        void apply(KeyPressEvent& keyPress) override
        {
            if (closeKey != KeySymbol::KEY_Undefined && keyPress.keyBase == closeKey) close();
        }

        void apply(CloseWindowEvent&) override
        {
            close();
        }

        void apply(TerminateEvent&) override
        {
            close();
        }

    protected:
        // use observer_ptr<> to avoid circular reference
        observer_ptr<Viewer> _viewer;
    };

} // namespace vsg
