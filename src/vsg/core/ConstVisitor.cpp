/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ConstVisitor.h>

#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>

#include <vsg/vk/Command.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/RenderPass.h>

using namespace vsg;

ConstVisitor::ConstVisitor()
{
}

void ConstVisitor::apply(const Object&)
{
}

////////////////////////////////////////////////////////////////////////////////
//
// Values
//
void ConstVisitor::apply(const stringValue& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const boolValue& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const intValue& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const uintValue& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const floatValue& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const doubleValue& value)
{
    apply(static_cast<const Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Arrays
//
void ConstVisitor::apply(const ubyteArray& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ushortArray& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const uintArray& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const floatArray& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const doubleArray& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec2Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec3Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec4Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const mat4Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec2Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec3Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec4Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dmat4Array& value)
{
    apply(static_cast<const Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Array2Ds
//
void ConstVisitor::apply(const ubyteArray2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ushortArray2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const uintArray2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const floatArray2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const doubleArray2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec2Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec3Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec4Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec2Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec3Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec4Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ubvec2Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ubvec3Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ubvec4Array2D& value)
{
    apply(static_cast<const Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Nodes
//
void ConstVisitor::apply(const Node& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const Group& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const QuadGroup& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const LOD& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const StateGroup& value)
{
    apply(static_cast<const Group&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Vulkan Object
//
void ConstVisitor::apply(const Command& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const CommandBuffer& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const RenderPass& value)
{
    apply(static_cast<const Object&>(value));
}
