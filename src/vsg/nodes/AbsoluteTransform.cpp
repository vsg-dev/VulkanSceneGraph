/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/AbsoluteTransform.h>

using namespace vsg;

AbsoluteTransform::AbsoluteTransform()
{
}

AbsoluteTransform::AbsoluteTransform(const AbsoluteTransform& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    matrix(rhs.matrix)
{
}

AbsoluteTransform::AbsoluteTransform(const dmat4& in_matrix) :
    matrix(in_matrix)
{
}

int AbsoluteTransform::compare(const Object& rhs_object) const
{
    int result = Transform::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_value(matrix, rhs.matrix);
}

void AbsoluteTransform::read(Input& input)
{
    if (input.version_greater_equal(1, 1, 2))
    {
        Node::read(input);
        input.read("matrix", matrix);
        input.read("subgraphRequiresLocalFrustum", subgraphRequiresLocalFrustum);
        input.readObjects("children", children);
    }
    else
    {
        Transform::read(input);
        input.read("matrix", matrix);
        input.read("subgraphRequiresLocalFrustum", subgraphRequiresLocalFrustum);
    }
}

void AbsoluteTransform::write(Output& output) const
{
    if (output.version_greater_equal(1, 1, 2))
    {
        Node::write(output);
        output.write("matrix", matrix);
        output.write("subgraphRequiresLocalFrustum", subgraphRequiresLocalFrustum);
        output.writeObjects("children", children);
    }
    else
    {
        Transform::write(output);
        output.write("matrix", matrix);
        output.write("subgraphRequiresLocalFrustum", subgraphRequiresLocalFrustum);
    }
}
