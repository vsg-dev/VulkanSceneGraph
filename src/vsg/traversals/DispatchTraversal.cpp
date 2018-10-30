/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/DispatchTraversal.h>

#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>

#include <vsg/vk/Command.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

#include <iostream>

using namespace vsg;

#define INLINE_TRAVERSE

struct DispatchTraversal::Data
{
    explicit Data(CommandBuffer* commandBuffer) :
        _commandBuffer(commandBuffer)
    {
        std::cout << "DispatchTraversal::Data::Data(" << commandBuffer << ")" << std::endl;
    }

    ~Data()
    {
        std::cout << "DispatchTraversal::Data::~Data()" << std::endl;
    }

    State _state;
    ref_ptr<CommandBuffer> _commandBuffer;
};

DispatchTraversal::DispatchTraversal(CommandBuffer* commandBuffer) :
    _data(new Data(commandBuffer))
{
}

DispatchTraversal::~DispatchTraversal()
{
    delete _data;
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

void DispatchTraversal::apply(const StateGroup& stateGroup)
{
    //    std::cout<<"Visiting StateGroup "<<std::endl;
    stateGroup.pushTo(_data->_state);

    stateGroup.traverse(*this);

    //    stateGroup.popFrom(_data->_state);
}

// Vulkan nodes
void DispatchTraversal::apply(const Command& command)
{
    //    std::cout<<"Visiting Command "<<std::endl;
    _data->_state.dispatch(*(_data->_commandBuffer));
    command.dispatch(*(_data->_commandBuffer));
}
