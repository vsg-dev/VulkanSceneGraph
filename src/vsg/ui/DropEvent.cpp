/* <editor-fold desc="MIT License">

Copyright(c) 2025 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/ui/DropEvent.h>

using namespace vsg;

void DropEvent::read(Input& input)
{
    UIEvent::read(input);

    input.read("x", x);
    input.read("y", y);
    input.readValues("paths", paths);
}

void DropEvent::write(Output& output) const
{
    UIEvent::write(output);

    output.write("x", x);
    output.write("y", y);
    output.writeValues("paths", paths);
}

void DropHoverEvent::read(Input& input)
{
    DropEvent::read(input);

    input.read("accept", accept);
}

void DropHoverEvent::write(Output& output) const
{
    DropEvent::write(output);

    output.write("accept", accept);
}
