/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/ViewMatrix.h>
#include <vsg/io/Options.h>
#include <vsg/maths/transform.h>

using namespace vsg;

void LookAt::read(Input& input)
{
    ViewMatrix::read(input);

    input.read("eye", eye);
    input.read("center", center);
    input.read("up", up);
}

void LookAt::write(Output& output) const
{
    ViewMatrix::write(output);

    output.write("eye", eye);
    output.write("center", center);
    output.write("up", up);
}

dmat4 TrackingViewMatrix::transform() const
{
    return matrix * vsg::inverse(computeTransform(objectPath));
}

dmat4 TrackingViewMatrix::inverse() const
{
    return computeTransform(objectPath) * vsg::inverse(matrix);
}
