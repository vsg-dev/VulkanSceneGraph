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

#include <iostream>

using namespace vsg;

#define INLINE_TRAVERSE

class DispatchTraversal::InternalData
{
public:
    State _state;
    ref_ptr<CommandBuffer> _commandBuffer;

    using Polytope = std::vector<vsg::plane>;

    Polytope _frustumUnit;

    bool _frustumDirty;
    Polytope _frustum;

    explicit InternalData(CommandBuffer* commandBuffer) :
        _commandBuffer(commandBuffer)
    {
        //        std::cout << "DispatchTraversal::InternalData::InternalData(" << commandBuffer << ")" << std::endl;
        _frustumUnit = Polytope{
            vsg::plane(1.0, 0.0, 0.0, 1.0),  // left plane
            vsg::plane(-1.0, 0.0, 0.0, 1.0), // right plane
            vsg::plane(0.0, 1.0, 0.0, 1.0),  // bottom plane
            vsg::plane(0.0, -1.0, 0.0, 1.0)  // top plane
        };

        _frustumDirty = true;
    }

    ~InternalData()
    {
        //        std::cout << "DispatchTraversal::InternalData::~InternalData()" << std::endl;
    }

    template<typename T>
    constexpr bool intersect(t_sphere<T> const& s)
    {
        if (_frustumDirty)
        {
            auto pmv = _state.projectionMatrixStack.top() * _state.viewMatrixStack.top() * _state.modelMatrixStack.top();

#if 0
            std::cout<<"   pmv = "<<pmv<<std::endl;
            std::cout<<"   s = "<<s.vec<<std::endl;
#endif
            _frustum.clear();
            for (auto& pl : _frustumUnit)
            {
                _frustum.push_back(pl * pmv);
            }

            _frustumDirty = false;
        }

        return vsg::intersect(_frustum, s);
    }
};

DispatchTraversal::DispatchTraversal(CommandBuffer* commandBuffer) :
    _data(new InternalData(commandBuffer))
{
}

DispatchTraversal::~DispatchTraversal()
{
    delete _data;
}

void DispatchTraversal::setProjectionMatrix(const dmat4& projMatrix)
{
    _data->_state.projectionMatrixStack.set(projMatrix);
}

void DispatchTraversal::setViewMatrix(const dmat4& viewMatrix)
{
    _data->_state.viewMatrixStack.set(viewMatrix);
}

void DispatchTraversal::apply(const Object& object)
{
    //    std::cout<<"Visiting object"<<std::endl;
    object.traverse(*this);
}

void DispatchTraversal::apply(const Group& group)
{
//    std::cout<<"Visiting Group "<<std::endl;
#ifdef INLINE_TRAVERSE
    vsg::Group::t_traverse(group, *this);
#else
    group.traverse(*this);
#endif
}

void DispatchTraversal::apply(const QuadGroup& group)
{
//    std::cout<<"Visiting QuadGroup "<<std::endl;
#ifdef INLINE_TRAVERSE
    vsg::QuadGroup::t_traverse(group, *this);
#else
    group.traverse(*this);
#endif
}

void DispatchTraversal::apply(const LOD& object)
{
    //    std::cout<<"Visiting LOD "<<std::endl;
    object.traverse(*this);
}

void DispatchTraversal::apply(const CullGroup& cullGroup)
{
#if 0
    // no culling
    cullGroup.traverse(*this);
#else
    if (_data->intersect(cullGroup.getBound()))
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
    cullGroup.traverse(*this);
#else
    if (_data->intersect(cullNode.getBound()))
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
    stateGroup.pushTo(_data->_state);

    stateGroup.traverse(*this);

    stateGroup.popFrom(_data->_state);
}

void DispatchTraversal::apply(const MatrixTransform& mt)
{
    _data->_state.modelMatrixStack.push(mt.getMatrix());
    _data->_state.dirty = true;

    mt.traverse(*this);

    _data->_state.modelMatrixStack.pop();
    _data->_state.dirty = true;
}

// Vulkan nodes
void DispatchTraversal::apply(const Commands& commands)
{
    //    std::cout<<"Visiting Command "<<std::endl;
    _data->_state.dispatch(*(_data->_commandBuffer));
    for (auto& command : commands.getChildren())
    {
        command->dispatch(*(_data->_commandBuffer));
    }
}

void DispatchTraversal::apply(const Command& command)
{
    //    std::cout<<"Visiting Command "<<std::endl;
    _data->_state.dispatch(*(_data->_commandBuffer));
    command.dispatch(*(_data->_commandBuffer));
}
