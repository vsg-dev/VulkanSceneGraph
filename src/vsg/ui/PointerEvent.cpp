/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/ui/PointerEvent.h>

using namespace vsg;

void PointerEvent::read(Input& input)
{
    UIEvent::read(input);

    input.read("x", x);
    input.read("y", y);
    input.readValue<uint16_t>("mask", mask);
}

void PointerEvent::write(Output& output) const
{
    UIEvent::write(output);

    output.write("x", x);
    output.write("y", y);
    output.writeValue<uint16_t>("mask", mask);
}

void ButtonPressEvent::read(Input& input)
{
    PointerEvent::read(input);

    input.read("button", button);
}

void ButtonPressEvent::write(Output& output) const
{
    PointerEvent::write(output);

    output.write("button", button);
}

void ButtonReleaseEvent::read(Input& input)
{
    PointerEvent::read(input);

    input.read("button", button);
}

void ButtonReleaseEvent::write(Output& output) const
{
    PointerEvent::write(output);

    output.write("button", button);
}
