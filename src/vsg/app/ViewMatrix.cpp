/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/ViewMatrix.h>
#include <vsg/maths/transform.h>

using namespace vsg;

void ViewMatrix::read(Input& input)
{
    Object::read(input);

    if (input.version_greater_equal(1, 1, 9))
        input.read("origin", origin);
    else
        origin = {};
}

void ViewMatrix::write(Output& output) const
{
    Object::write(output);

    if (output.version_greater_equal(1, 1, 9))
        output.write("origin", origin);
}

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

void LookAt::transform(const dmat4& matrix)
{
    up = normalize(matrix * (eye + up) - matrix * eye);
    center = matrix * center;
    eye = matrix * eye;
}

void LookAt::set(const dmat4& matrix)
{
    up = normalize(matrix * (dvec3(0.0, 0.0, 0.0) + dvec3(0.0, 1.0, 0.0)) - matrix * dvec3(0.0, 0.0, 0.0));
    center = matrix * dvec3(0.0, 0.0, -1.0);
    eye = matrix * dvec3(0.0, 0.0, 0.0);
}

dmat4 LookAt::transform(const dvec3& offset) const
{
    dvec3 delta = dvec3(origin - offset);
    return vsg::lookAt(eye + delta, center + delta, up);
}

void LookDirection::set(const dmat4& matrix)
{
    dvec3 scale;
    vsg::decompose(matrix, position, rotation, scale);
}

dmat4 LookDirection::transform(const dvec3& offset) const
{
    return vsg::rotate(-rotation) * vsg::translate(dvec3(offset - origin) - position);
}

dmat4 RelativeViewMatrix::transform(const dvec3& offset) const
{
    return matrix * viewMatrix->transform(offset);
}

dmat4 TrackingViewMatrix::transform(const dvec3& offset) const
{
    return matrix * vsg::translate(dvec3(offset - origin)) * vsg::inverse(computeTransform(objectPath));
}

dmat4 TrackingViewMatrix::inverse(const dvec3& offset) const
{
    return vsg::computeTransform(objectPath) * vsg::translate(dvec3(offset - origin)) * vsg::inverse(matrix);
}
