/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Commands.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/traversals/ComputeBounds.h>

using namespace vsg;

ComputeBounds::ComputeBounds(ref_ptr<ArrayState> initialArrayData)
{
    arrayStateStack.reserve(4);
    arrayStateStack.emplace_back(initialArrayData ? initialArrayData : ArrayState::create());
}

void ComputeBounds::apply(const vsg::Object& object)
{
    object.traverse(*this);
}

void ComputeBounds::apply(const StateGroup& stategroup)
{
    auto arrayState = stategroup.prototypeArrayState ? stategroup.prototypeArrayState->clone(arrayStateStack.back()) : arrayStateStack.back()->clone();

    for (auto& statecommand : stategroup.stateCommands)
    {
        statecommand->accept(*arrayState);
    }

    arrayStateStack.emplace_back(arrayState);

    stategroup.traverse(*this);

    arrayStateStack.pop_back();
}

void ComputeBounds::apply(const Transform& transform)
{
    if (matrixStack.empty())
        matrixStack.push_back(transform.transform(dmat4{}));
    else
        matrixStack.push_back(transform.transform(matrixStack.back()));

    transform.traverse(*this);

    matrixStack.pop_back();
}

void ComputeBounds::apply(const MatrixTransform& transform)
{
    if (matrixStack.empty())
        matrixStack.push_back(transform.matrix);
    else
        matrixStack.push_back(matrixStack.back() * transform.matrix);

    transform.traverse(*this);

    matrixStack.pop_back();
}

void ComputeBounds::apply(const vsg::Geometry& geometry)
{
    auto& arrayState = *arrayStateStack.back();
    arrayState.apply(geometry);
    if (arrayState.vertices) apply(*arrayState.vertices);
}

void ComputeBounds::apply(const vsg::VertexIndexDraw& vid)
{
    auto& arrayState = *arrayStateStack.back();
    arrayState.apply(vid);
    if (arrayState.vertices) apply(*arrayState.vertices);
}

void ComputeBounds::apply(const vsg::BindVertexBuffers& bvb)
{
    auto& arrayState = *arrayStateStack.back();
    arrayState.apply(bvb);
    if (arrayState.vertices) apply(*arrayState.vertices);
}

void ComputeBounds::apply(const vsg::StateCommand& statecommand)
{
    statecommand.accept(*arrayStateStack.back());
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
        for (auto vertex : vertices) bounds.add(matrix * dvec3(vertex));
    }
}
