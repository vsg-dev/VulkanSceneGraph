#include <vsg/core/Visitor.h>
#include <vsg/core/Value.h>

#include <vsg/nodes/Node.h>
#include <vsg/nodes/Group.h>

using namespace vsg;

Visitor::Visitor()
{
}

void Visitor::apply(Object&)
{
}

void Visitor::apply(StringValue& value)
{
    apply(static_cast<Object&>(value));
}

void Visitor::apply(IntValue& value)
{
    apply(static_cast<Object&>(value));
}

void Visitor::apply(FloatValue& value)
{
    apply(static_cast<Object&>(value));
}

void Visitor::apply(DoubleValue& value)
{
    apply(static_cast<Object&>(value));
}

void Visitor::apply(Node& node)
{
    apply(static_cast<Object&>(node));
}

void Visitor::apply(Group& group)
{
    apply(static_cast<Node&>(group));
}

