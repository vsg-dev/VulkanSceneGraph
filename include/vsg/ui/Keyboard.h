#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2023 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/KeyEvent.h>

namespace vsg
{

    /// Keyboard tracks keyboard events to maintain the key pressed state and how long the key has been hel for
    class VSG_DECLSPEC Keyboard : public Inherit<Visitor, Keyboard>
    {
    public:
        void apply(KeyPressEvent& keyPress) override;
        void apply(KeyReleaseEvent& keyRelease) override;
        void apply(FocusInEvent& focusIn) override;
        void apply(FocusOutEvent& focusOut) override;

        struct KeyHistory
        {
            vsg::time_point timeOfFirstKeyPress = {};
            vsg::time_point timeOfLastKeyPress = {};
            vsg::time_point timeOfKeyRelease = {};
            bool handled = false;
        };

        std::map<KeySymbol, KeyHistory> keyState;

        /// return true if key is currently pressed
        bool pressed(KeySymbol key, bool ignore_handled_keys = true) const;

        /// return a pair of times, the first is the time, in seconds, since the key was first pressed and the second is the time, in secnds, since it was released.
        /// if the key hasn't been pressed then then first value will be < 0.0, if the key is still pressed then the second value will be 0.0.
        std::pair<double, double> times(KeySymbol key, bool ignore_handled_keys = true) const;
    };
    VSG_type_name(vsg::Keyboard);

} // namespace vsg
