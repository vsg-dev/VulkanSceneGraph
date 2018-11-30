/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Visitor.h>

#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>

#include <vsg/vk/Command.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/RenderPass.h>

#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>

using namespace vsg;

Visitor::Visitor()
{
}

void Visitor::apply(Object&)
{
}

////////////////////////////////////////////////////////////////////////////////
//
// Values
//
void Visitor::apply(stringValue& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(boolValue& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(intValue& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(uintValue& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(floatValue& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(doubleValue& value)
{
    apply(static_cast<Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Arrays
//
void Visitor::apply(ubyteArray& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ushortArray& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(uintArray& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(floatArray& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(doubleArray& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(vec2Array& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(vec3Array& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(vec4Array& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(dvec2Array& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(dvec3Array& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(dvec4Array& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ubvec2Array& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ubvec3Array& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ubvec4Array& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(mat4Array& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(dmat4Array& value)
{
    apply(static_cast<Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Array2Ds
//
void Visitor::apply(ubyteArray2D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ushortArray2D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(uintArray2D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(floatArray2D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(doubleArray2D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(vec2Array2D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(vec3Array2D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(vec4Array2D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(dvec2Array2D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(dvec3Array2D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(dvec4Array2D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ubvec2Array2D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ubvec3Array2D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ubvec4Array2D& value)
{
    apply(static_cast<Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Array3Ds
//
void Visitor::apply(ubyteArray3D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ushortArray3D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(uintArray3D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(floatArray3D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(doubleArray3D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(vec2Array3D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(vec3Array3D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(vec4Array3D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(dvec2Array3D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(dvec3Array3D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(dvec4Array3D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ubvec2Array3D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ubvec3Array3D& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ubvec4Array3D& value)
{
    apply(static_cast<Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Nodes
//
void Visitor::apply(Node& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(Group& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(QuadGroup& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(LOD& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(StateGroup& value)
{
    apply(static_cast<Group&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Vulkan Object
//
void Visitor::apply(Command& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(CommandBuffer& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(RenderPass& value)
{
    apply(static_cast<Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// UI Events
//
void Visitor::apply(UIEvent& event)
{
    apply(static_cast<Object&>(event));
}
void Visitor::apply(WindowEvent& event)
{
    apply(static_cast<UIEvent&>(event));
}
void Visitor::apply(ExposeWindowEvent& event)
{
    apply(static_cast<WindowEvent&>(event));
}
void Visitor::apply(DeleteWindowEvent& event)
{
    apply(static_cast<WindowEvent&>(event));
}
void Visitor::apply(KeyEvent& event)
{
    apply(static_cast<WindowEvent&>(event));
}
void Visitor::apply(KeyPressEvent& event)
{
    apply(static_cast<KeyEvent&>(event));
}
void Visitor::apply(KeyReleaseEvent& event)
{
    apply(static_cast<KeyEvent&>(event));
}
void Visitor::apply(PointerEvent& event)
{
    apply(static_cast<WindowEvent&>(event));
}
void Visitor::apply(ButtonPressEvent& event)
{
    apply(static_cast<PointerEvent&>(event));
}
void Visitor::apply(ButtonReleaseEvent& event)
{
    apply(static_cast<PointerEvent&>(event));
}
void Visitor::apply(MoveEvent& event)
{
    apply(static_cast<PointerEvent&>(event));
}

