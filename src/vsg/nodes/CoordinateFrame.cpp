/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/io/stream.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/CoordinateFrame.h>

using namespace vsg;

CoordinateFrame::CoordinateFrame()
{
}

CoordinateFrame::CoordinateFrame(const CoordinateFrame& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    name(rhs.name),
    origin(rhs.origin),
    rotation(rhs.rotation)
{
}

int CoordinateFrame::compare(const Object& rhs_object) const
{
    int result = Transform::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(name, rhs.name)) != 0) return result;
    if ((result = compare_value(origin, rhs.origin)) != 0) return result;
    return compare_value(rotation, rhs.rotation);
}

void CoordinateFrame::read(Input& input)
{
    Node::read(input);
    input.read("name", name);
    input.read("origin", origin);
    input.read("rotation", rotation);
    input.read("subgraphRequiresLocalFrustum", subgraphRequiresLocalFrustum);
    input.readObjects("children", children);
}

void CoordinateFrame::write(Output& output) const
{
    Node::write(output);
    output.write("name", name);
    output.write("origin", origin);
    output.write("rotation", rotation);
    output.write("subgraphRequiresLocalFrustum", subgraphRequiresLocalFrustum);
    output.writeObjects("children", children);
}

dmat4 CoordinateFrame::transform(const dmat4& mv) const
{
    return mv * translate(dvec3(origin)) * rotate(rotation);
}
