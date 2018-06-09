#include <vsg/Visitor.h>
#include <vsg/Group.h>

using namespace vsg;

Visitor::Visitor()
{
}

void Visitor::apply(Object& object)
{
}

void Visitor::apply(Node& node)
{
    apply(static_cast<Object&>(node));
}

void Visitor::apply(Group& group)
{
    apply(static_cast<Node&>(group));
}

