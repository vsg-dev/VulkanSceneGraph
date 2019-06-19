/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/DispatchTraversal.h>

#include <vsg/nodes/Commands.h>
#include <vsg/nodes/CullGroup.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>

#include <vsg/vk/Command.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

#include <vsg/maths/plane.h>

using namespace vsg;

#define INLINE_TRAVERSE 1
#define USE_FRUSTUM_ARRAY 1

DispatchTraversal::DispatchTraversal(CommandBuffer* commandBuffer, uint32_t maxSlot) :
    _state(new State(commandBuffer, maxSlot))
{
}

DispatchTraversal::~DispatchTraversal()
{
    delete _state;
}

void DispatchTraversal::setProjectionAndViewMatrix(const dmat4& projMatrix, const dmat4& viewMatrix)
{
    _state->setProjectionAndViewMatrix(projMatrix, viewMatrix);
}

void DispatchTraversal::apply(const Object& object)
{
    //    std::cout<<"Visiting object"<<std::endl;
    object.traverse(*this);
}

void DispatchTraversal::apply(const Group& group)
{
//    std::cout<<"Visiting Group "<<std::endl;
#if INLINE_TRAVERSE
    vsg::Group::t_traverse(group, *this);
#else
    group.traverse(*this);
#endif
}

void DispatchTraversal::apply(const QuadGroup& group)
{
//    std::cout<<"Visiting QuadGroup "<<std::endl;
#if INLINE_TRAVERSE
    vsg::QuadGroup::t_traverse(group, *this);
#else
    group.traverse(*this);
#endif
}

void DispatchTraversal::apply(const LOD& lod)
{
    auto sphere = lod.getBound();

    // check if lod bounding sphere is in vie frustum.
    if (!_state->intersect(sphere))
    {
        return;
    }

    const auto& proj = _state->projectionMatrixStack.top();
    const auto& mv = _state->modelviewMatrixStack.top();
    auto f = -proj[1][1];
    vsg::vec4 lv(mv[0][2], mv[1][2], mv[2][2], mv[3][2]);

    auto distance = std::abs(lv.x * sphere.x + lv.y * sphere.y + lv.z * sphere.z + lv.w);
    auto rf = sphere.r * f;

    for (auto lodChild : lod.getChildren())
    {
        bool child_visible = rf > (lodChild.minimumScreenHeightRatio * distance);
        if (child_visible)
        {
            lodChild.child->accept(*this);
            return;
        }
    }
}

void DispatchTraversal::apply(const CullGroup& cullGroup)
{
#if 0
    // no culling
    cullGroup.traverse(*this);
#else
    if (_state->intersect(cullGroup.getBound()))
    {
        //std::cout<<"Passed node"<<std::endl;
        cullGroup.traverse(*this);
    }
    else
    {
        //std::cout<<"Culling node"<<std::endl;
    }
#endif
}

void DispatchTraversal::apply(const CullNode& cullNode)
{
#if 0
    // no culling
    cullNode.traverse(*this);
#else
    if (_state->intersect(cullNode.getBound()))
    {
        //std::cout<<"Passed node"<<std::endl;
        cullNode.traverse(*this);
    }
    else
    {
        //std::cout<<"Culling node"<<std::endl;
    }
#endif
}

void DispatchTraversal::apply(const StateGroup& stateGroup)
{
    //    std::cout<<"Visiting StateGroup "<<std::endl;

    const StateGroup::StateCommands& stateCommands = stateGroup.getStateCommands();
    for (auto& command : stateCommands)
    {
        _state->stateStacks[command->getSlot()].push(command);
    }
    _state->dirty = true;

    stateGroup.traverse(*this);

    for (auto& command : stateCommands)
    {
        _state->stateStacks[command->getSlot()].pop();
    }
    _state->dirty = true;
}

void DispatchTraversal::apply(const MatrixTransform& mt)
{
    if (mt.getSubgraphRequiresLocalFrustum())
    {
        _state->modelviewMatrixStack.pushAndPreMult(mt.getMatrix());
        _state->pushFrustum();
        _state->dirty = true;

        mt.traverse(*this);

        _state->modelviewMatrixStack.pop();
        _state->popFrustum();
        _state->dirty = true;
    }
    else
    {
        _state->modelviewMatrixStack.pushAndPreMult(mt.getMatrix());
        _state->dirty = true;

        mt.traverse(*this);

        _state->modelviewMatrixStack.pop();
        _state->dirty = true;
    }
}

// Vulkan nodes
void DispatchTraversal::apply(const Commands& commands)
{
    _state->dispatch();
    for (auto& command : commands.getChildren())
    {
        command->dispatch(*(_state->_commandBuffer));
    }
}

void DispatchTraversal::apply(const Command& command)
{
    //    std::cout<<"Visiting Command "<<std::endl;
    _state->dispatch();
    command.dispatch(*(_state->_commandBuffer));
}
