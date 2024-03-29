#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/ui/UIEvent.h>

namespace vsg
{

    /// FrameStamp represents the time and frame count of a specific frame.
    class VSG_DECLSPEC FrameStamp : public Inherit<Object, FrameStamp>
    {
    public:
        FrameStamp() {}

        FrameStamp(time_point in_time, uint64_t in_frameCount, double in_simulationTime) :
            time(in_time),
            frameCount(in_frameCount),
            simulationTime(in_simulationTime) {}

        time_point time = {};
        uint64_t frameCount = 0;
        double simulationTime = 0.0;

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::FrameStamp);

} // namespace vsg
