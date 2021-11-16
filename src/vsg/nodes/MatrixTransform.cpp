/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/io/stream.h>
#include <vsg/nodes/MatrixTransform.h>

using namespace vsg;

MatrixTransform::MatrixTransform(Allocator* allocator) :
    Inherit(allocator)
{
}

MatrixTransform::MatrixTransform(const dmat4& in_matrix, Allocator* allocator) :
    Inherit(allocator),
    matrix(in_matrix)
{
}

void MatrixTransform::read(Input& input)
{
    Transform::read(input);

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("matrix", matrix);
        input.read("subgraphRequiresLocalFrustum", subgraphRequiresLocalFrustum);
    }
    else
    {
        input.read("Matrix", matrix);
        input.read("SubgraphRequiresLocalFrustum", subgraphRequiresLocalFrustum);
    }
}

void MatrixTransform::write(Output& output) const
{
    Transform::write(output);

    if (output.version_greater_equal(0, 1, 4))
    {
        output.write("matrix", matrix);
        output.write("subgraphRequiresLocalFrustum", subgraphRequiresLocalFrustum);
    }
    else
    {
        output.write("Matrix", matrix);
        output.write("SubgraphRequiresLocalFrustum", subgraphRequiresLocalFrustum);
    }
}
