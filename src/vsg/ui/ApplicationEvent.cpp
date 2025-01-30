/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/ui/ApplicationEvent.h>

using namespace vsg;

void FrameStamp::read(Input& input)
{
    Object::read(input);

    uint64_t time_since_epoch;
    input.readValue<uint64_t>("time", time_since_epoch);
    time = clock::time_point(clock::time_point::duration(time_since_epoch));

    input.read("frameCount", frameCount);

    if (input.version_greater_equal(1, 1, 2))
    {
        input.read("simulationTime", simulationTime);
    }
}

void FrameStamp::write(Output& output) const
{
    Object::write(output);

    uint64_t time_since_epoch = time.time_since_epoch().count();
    output.writeValue<uint64_t>("time", time_since_epoch);

    output.write("frameCount", frameCount);

    if (output.version_greater_equal(1, 1, 2))
    {
        output.write("simulationTime", simulationTime);
    }
}

void FrameEvent::read(Input& input)
{
    UIEvent::read(input);

    input.readObject("frameStamp", frameStamp);
}

void FrameEvent::write(Output& output) const
{
    UIEvent::write(output);

    output.writeObject("frameStamp", frameStamp);
}
