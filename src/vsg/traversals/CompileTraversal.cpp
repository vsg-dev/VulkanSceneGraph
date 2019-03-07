/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>

#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/Geometry.h>

#include <vsg/vk/Command.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

#include <iostream>

using namespace vsg;

CompileTraversal::CompileTraversal()
{
}

CompileTraversal::~CompileTraversal()
{
}

void CompileTraversal::apply(Object& object)
{
    object.traverse(*this);
}

void CompileTraversal::apply(Command& command)
{
    command.compile(context);
}

void CompileTraversal::apply(StateGroup& stateGroup)
{
    stateGroup.compile(context);
    stateGroup.traverse(*this);
}

void CompileTraversal::apply(Geometry& geometry)
{
    geometry.compile(context);
    geometry.traverse(*this);
}
