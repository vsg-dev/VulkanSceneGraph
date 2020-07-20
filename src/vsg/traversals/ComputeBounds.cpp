/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Commands.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/traversals/ComputeBounds.h>
#include <vsg/io/Options.h>


using namespace vsg;

ComputeBounds::ComputeBounds()
{
}

void ComputeBounds::apply(const vsg::Node& node)
{
    node.traverse(*this);
}

void ComputeBounds::apply(const vsg::Group& group)
{
    group.traverse(*this);
}

void ComputeBounds::apply(const vsg::MatrixTransform& transform)
{
    matrixStack.push_back(transform.getMatrix());

    transform.traverse(*this);

    matrixStack.pop_back();
}

void ComputeBounds::apply(const vsg::Geometry& geometry)
{
    if (!geometry.arrays.empty())
    {
        geometry.arrays[0]->accept(*this);
    }
}

void ComputeBounds::apply(const vsg::Commands& commands)
{
    commands.traverse(*this);
}

void ComputeBounds::apply(const vsg::VertexIndexDraw& vid)
{
    if (!vid.arrays.empty())
    {
        vid.arrays[0]->accept(*this);
    }
}

void ComputeBounds::apply(const vsg::BindVertexBuffers& bvb)
{
    if (!bvb.getArrays().empty())
    {
        bvb.getArrays()[0]->accept(*this);
    }
}

void ComputeBounds::apply(const vsg::vec3Array& vertices)
{
    if (matrixStack.empty())
    {
        for (auto vertex : vertices) bounds.add(vertex);
    }
    else
    {
        auto matrix = matrixStack.back();
        for (auto vertex : vertices) bounds.add(matrix * vertex);
    }
}
