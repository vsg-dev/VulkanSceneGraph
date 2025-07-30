/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/ProjectionMatrix.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Perspective
//
void Perspective::read(Input& input)
{
    ProjectionMatrix::read(input);

    input.read("fieldOfViewY", fieldOfViewY);
    input.read("aspectRatio", aspectRatio);
    input.read("nearDistance", nearDistance);
    input.read("farDistance", farDistance);
}

void Perspective::write(Output& output) const
{
    ProjectionMatrix::write(output);

    output.write("fieldOfViewY", fieldOfViewY);
    output.write("aspectRatio", aspectRatio);
    output.write("nearDistance", nearDistance);
    output.write("farDistance", farDistance);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Orthographic
//
void Orthographic::read(Input& input)
{
    ProjectionMatrix::read(input);

    input.read("left", left);
    input.read("right", right);
    input.read("bottom", bottom);
    input.read("top", top);
    input.read("nearDistance", nearDistance);
    input.read("farDistance", farDistance);
}

void Orthographic::write(Output& output) const
{
    ProjectionMatrix::write(output);

    output.write("left", left);
    output.write("right", right);
    output.write("bottom", bottom);
    output.write("top", top);
    output.write("nearDistance", nearDistance);
    output.write("farDistance", farDistance);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EllipsoidPerspective
//
void EllipsoidPerspective::read(Input& input)
{
    ProjectionMatrix::read(input);

    input.read("lookAt", lookAt);
    input.read("ellipsoidModel", ellipsoidModel);
    input.read("fieldOfViewY", fieldOfViewY);
    input.read("aspectRatio", aspectRatio);
    input.read("nearFarRatio", nearFarRatio);
    input.read("horizonMountainHeight", horizonMountainHeight);
}

void EllipsoidPerspective::write(Output& output) const
{
    ProjectionMatrix::write(output);

    output.write("lookAt", lookAt);
    output.write("ellipsoidModel", ellipsoidModel);
    output.write("fieldOfViewY", fieldOfViewY);
    output.write("aspectRatio", aspectRatio);
    output.write("nearFarRatio", nearFarRatio);
    output.write("horizonMountainHeight", horizonMountainHeight);
}
