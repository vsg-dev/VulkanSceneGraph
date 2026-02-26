#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2026 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/ui/FrameStamp.h>

namespace vsg
{

    // forward declare
    class Viewer;

    /// LoadBalancing is a base class for specifying the Camera view matrix and its inverse.
    class VSG_DECLSPEC LoadBalancing : public Inherit<Object, LoadBalancing>
    {
    public:
        LoadBalancing();
        explicit LoadBalancing(const LoadBalancing& rhs, const CopyOp& copyop = {});

        virtual void update(Viewer& viewer);

        time_point previousFrameTime = std::numeric_limits<time_point>::max();

        double targetFrameTime = 0.016; // seconds
        double targetCPUTime = 0.008; // seconds
        double targetGPUTime = 0.008; // seconds
        double targetGPUMemoryUtilization = 0.9; // ratio

    protected:
        ~LoadBalancing();
    };
    VSG_type_name(vsg::LoadBalancing);


} // namespace vsg
