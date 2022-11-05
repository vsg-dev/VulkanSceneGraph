/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Draw.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/io/Logger.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/Transform.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/utils/Intersector.h>

using namespace vsg;

struct PushPopNode
{
    Intersector::NodePath& nodePath;

    PushPopNode(Intersector::NodePath& np, const Node* node) :
        nodePath(np) { nodePath.push_back(node); }
    ~PushPopNode() { nodePath.pop_back(); }
};

Intersector::Intersector(ref_ptr<ArrayState> intialArrayState)
{
    arrayStateStack.reserve(4);
    arrayStateStack.emplace_back(intialArrayState ? intialArrayState : ArrayState::create());
}

void Intersector::apply(const Node& node)
{
    PushPopNode ppn(_nodePath, &node);

    node.traverse(*this);
}

void Intersector::apply(const StateGroup& stategroup)
{
    PushPopNode ppn(_nodePath, &stategroup);

    auto arrayState = stategroup.prototypeArrayState ? stategroup.prototypeArrayState->clone(arrayStateStack.back()) : arrayStateStack.back()->clone();

    for (auto& statecommand : stategroup.stateCommands)
    {
        statecommand->accept(*arrayState);
    }

    arrayStateStack.emplace_back(arrayState);

    stategroup.traverse(*this);

    arrayStateStack.pop_back();
}

void Intersector::apply(const Transform& transform)
{
    PushPopNode ppn(_nodePath, &transform);

    pushTransform(transform);

    transform.traverse(*this);

    popTransform();
}

void Intersector::apply(const LOD& lod)
{
    PushPopNode ppn(_nodePath, &lod);

    if (intersects(lod.bound))
    {
        for (auto& child : lod.children)
        {
            if (child.node)
            {
                child.node->accept(*this);
                break;
            }
        }
    }
}

void Intersector::apply(const PagedLOD& plod)
{
    PushPopNode ppn(_nodePath, &plod);

    if (intersects(plod.bound))
    {
        for (auto& child : plod.children)
        {
            if (child.node)
            {
                child.node->accept(*this);
                break;
            }
        }
    }
}

void Intersector::apply(const CullNode& cn)
{
    PushPopNode ppn(_nodePath, &cn);

    if (intersects(cn.bound)) cn.traverse(*this);
}

void Intersector::apply(const VertexIndexDraw& vid)
{
    auto& arrayState = *arrayStateStack.back();
    arrayState.apply(vid);
    if (!arrayState.vertices) return;

    if (vid.indices) vid.indices->accept(*this);

    PushPopNode ppn(_nodePath, &vid);

    intersectDrawIndexed(vid.firstIndex, vid.indexCount, vid.firstInstance, vid.instanceCount);
}

void Intersector::apply(const Geometry& geometry)
{
    auto& arrayState = *arrayStateStack.back();
    arrayState.apply(geometry);
    if (!arrayState.vertices) return;

    if (geometry.indices) geometry.indices->accept(*this);

    PushPopNode ppn(_nodePath, &geometry);

    for (auto& command : geometry.commands)
    {
        command->accept(*this);
    }
}

void Intersector::apply(const BindVertexBuffers& bvb)
{
    arrayStateStack.back()->apply(bvb);
}

void Intersector::apply(const BindIndexBuffer& bib)
{
    bib.indices->accept(*this);
}

void Intersector::apply(const BufferInfo& bufferInfo)
{
    if (bufferInfo.data) bufferInfo.data->accept(*this);
}

void Intersector::apply(const ushortArray& array)
{
    ushort_indices = &array;
    uint_indices = nullptr;
}

void Intersector::apply(const uintArray& array)
{
    ushort_indices = nullptr;
    uint_indices = &array;
}

void Intersector::apply(const Draw& draw)
{
    PushPopNode ppn(_nodePath, &draw);

    intersectDraw(draw.firstVertex, draw.vertexCount, draw.firstInstance, draw.instanceCount);
}

void Intersector::apply(const DrawIndexed& drawIndexed)
{
    PushPopNode ppn(_nodePath, &drawIndexed);

    intersectDrawIndexed(drawIndexed.firstIndex, drawIndexed.indexCount, drawIndexed.firstInstance, drawIndexed.instanceCount);
}
