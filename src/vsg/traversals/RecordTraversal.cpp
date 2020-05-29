/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/Command.h>
#include <vsg/commands/Commands.h>
#include <vsg/io/DatabasePager.h>
#include <vsg/io/Options.h>
#include <vsg/maths/plane.h>
#include <vsg/nodes/CullGroup.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/state/StateGroup.h>
#include <vsg/threading/atomics.h>
#include <vsg/traversals/RecordTraversal.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

using namespace vsg;

#include <iostream>

#define INLINE_TRAVERSE 1
#define USE_FRUSTUM_ARRAY 1

RecordTraversal::RecordTraversal(CommandBuffer* commandBuffer, uint32_t maxSlot, FrameStamp* fs) :
    _frameStamp(fs),
    _state(new State(commandBuffer, maxSlot))
{
    if (_frameStamp) _frameStamp->ref();
    if (_state)_state->ref();
}

RecordTraversal::~RecordTraversal()
{
    if (_culledPagedLODs) _culledPagedLODs->unref();
    if (_databasePager) _databasePager->unref();
    if (_state) _state->unref();
    if (_frameStamp) _frameStamp->unref();
}

void RecordTraversal::setFrameStamp(FrameStamp* fs)
{
    if (fs == _frameStamp) return;

    if (_frameStamp) _frameStamp->unref();

    _frameStamp = fs;

    if (_frameStamp) _frameStamp->ref();
}

void RecordTraversal::setDatabasePager(DatabasePager* dp)
{
    if (dp == _databasePager) return;

    if (_databasePager) _databasePager->unref();
    if (_culledPagedLODs) _culledPagedLODs->unref();

    _databasePager = dp;
    _culledPagedLODs = dp ? _databasePager->culledPagedLODs.get() : nullptr;

    if (_databasePager) _databasePager->ref();
    if (_culledPagedLODs) _culledPagedLODs->ref();
}

void RecordTraversal::setProjectionAndViewMatrix(const dmat4& projMatrix, const dmat4& viewMatrix)
{
    _state->setProjectionAndViewMatrix(projMatrix, viewMatrix);
}


void RecordTraversal::apply(const Object& object)
{
    //    std::cout<<"Visiting object"<<std::endl;
    object.traverse(*this);
}

void RecordTraversal::apply(const Group& group)
{
//    std::cout<<"Visiting Group "<<std::endl;
#if INLINE_TRAVERSE
    vsg::Group::t_traverse(group, *this);
#else
    group.traverse(*this);
#endif
}

void RecordTraversal::apply(const QuadGroup& group)
{
//    std::cout<<"Visiting QuadGroup "<<std::endl;
#if INLINE_TRAVERSE
    vsg::QuadGroup::t_traverse(group, *this);
#else
    group.traverse(*this);
#endif
}

void RecordTraversal::apply(const LOD& lod)
{
    auto sphere = lod.getBound();

    // check if lod bounding sphere is in view frustum.
    if (!_state->intersect(sphere))
    {
        return;
    }

    const auto& proj = _state->projectionMatrixStack.top();
    const auto& mv = _state->modelviewMatrixStack.top();
    auto f = -proj[1][1];

    auto distance = std::abs(mv[0][2] * sphere.x + mv[1][2] * sphere.y + mv[2][2] * sphere.z + mv[3][2]);
    auto rf = sphere.r * f;

    for (auto child : lod.getChildren())
    {
        bool child_visible = rf > (child.minimumScreenHeightRatio * distance);
        if (child_visible)
        {
            child.node->accept(*this);
            return;
        }
    }
}

void RecordTraversal::apply(const PagedLOD& plod)
{
    auto sphere = plod.getBound();

    auto frameCount = _frameStamp->frameCount;

    // check if lod bounding sphere is in view frustum.
    if (!_state->intersect(sphere))
    {
        if ((frameCount - plod.frameHighResLastUsed) > 1 && _culledPagedLODs)
        {
            _culledPagedLODs->highresCulled.emplace_back(&plod);
        }

        return;
    }

    const auto& proj = _state->projectionMatrixStack.top();
    const auto& mv = _state->modelviewMatrixStack.top();
    auto f = -proj[1][1];

    auto distance = std::abs(mv[0][2] * sphere.x + mv[1][2] * sphere.y + mv[2][2] * sphere.z + mv[3][2]);
    auto rf = sphere.r * f;

    // check the high res child to see if it's visible
    {
        const auto& child = plod.getChild(0);
        auto cutoff = child.minimumScreenHeightRatio * distance;
        bool child_visible = rf > cutoff;
        if (child_visible)
        {
            auto previousHighResUsed = plod.frameHighResLastUsed.exchange(frameCount);
            if (_culledPagedLODs && ((frameCount - previousHighResUsed) > 1))
            {
                _culledPagedLODs->newHighresRequired.emplace_back(&plod);
            }

            if (child.node)
            {
                // high res visible and availably so traverse it
                child.node->accept(*this);
                return;
            }
            else if (_databasePager)
            {
                auto priority = rf / cutoff;
                exchange_if_greater(plod.priority, priority);

                auto previousRequestCount = plod.requestCount.fetch_add(1);
                if (previousRequestCount == 0)
                {
                    // we are first request so tell the databasePager about it
                    _databasePager->request(ref_ptr<PagedLOD>(const_cast<PagedLOD*>(&plod)));
                }
                else
                {
                    //std::cout<<"repeat request "<<&plod<<", "<<plod.requestCount.load()<<std::endl;;
                }
            }
        }
        else
        {
            if (_culledPagedLODs && ((frameCount - plod.frameHighResLastUsed) <= 1))
            {
                _culledPagedLODs->highresCulled.emplace_back(&plod);
            }
        }
    }

    // check the low res child to see if it's visible
    {
        const auto& child = plod.getChild(1);
        auto cutoff = child.minimumScreenHeightRatio * distance;
        bool child_visible = rf > cutoff;
        if (child_visible)
        {
            if (child.node)
            {
                child.node->accept(*this);
            }
        }
    }
}

void RecordTraversal::apply(const CullGroup& cullGroup)
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

void RecordTraversal::apply(const CullNode& cullNode)
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

void RecordTraversal::apply(const StateGroup& stateGroup)
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

void RecordTraversal::apply(const MatrixTransform& mt)
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
void RecordTraversal::apply(const Commands& commands)
{
    _state->dispatch();
    for (auto& command : commands.getChildren())
    {
        command->dispatch(*(_state->_commandBuffer));
    }
}

void RecordTraversal::apply(const Command& command)
{
    //    std::cout<<"Visiting Command "<<std::endl;
    _state->dispatch();
    command.dispatch(*(_state->_commandBuffer));
}
