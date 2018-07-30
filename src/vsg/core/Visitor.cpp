#include <vsg/core/Visitor.h>

#include <vsg/nodes/Node.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/LOD.h>

#include <vsg/vk/Command.h>
#include <vsg/vk/RenderPass.h>

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
void Visitor::apply(StringValue& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(IntValue& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(UIntValue& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(FloatValue& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(DoubleValue& value) {apply(static_cast<Object&>(value)); }

////////////////////////////////////////////////////////////////////////////////
//
// Arrays
//
void Visitor::apply(ushortArray& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(uintArray& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(floatArray& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(doubleArray& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(vec2Array& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(vec3Array& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(vec4Array& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(mat4Array& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(dvec2Array& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(dvec3Array& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(dvec4Array& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(dmat4Array& value) { apply(static_cast<Object&>(value)); }


////////////////////////////////////////////////////////////////////////////////
//
// Nodes
//
void Visitor::apply(Node& value) { apply(static_cast<Object&>(value)); }
void Visitor::apply(Group& value) { apply(static_cast<Node&>(value)); }
void Visitor::apply(QuadGroup& value) { apply(static_cast<Node&>(value)); }
void Visitor::apply(LOD& value) { apply(static_cast<Node&>(value)); }

////////////////////////////////////////////////////////////////////////////////
//
// Vulkan Nodes
//
void Visitor::apply(Command& value) { apply(static_cast<Node&>(value)); }
void Visitor::apply(RenderPass& value) { apply(static_cast<Group&>(value)); }
