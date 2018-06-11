#include <vsg/nodes/Group.h>

#include <iostream>

using namespace vsg;

Group::Group()
{
//    std::cout<<"Group::Group() "<<this<<std::endl;
}

Group::~Group()
{
//    std::cout<<"Group::~Group() "<<this<<std::endl;
}

#ifndef INLINE_TRAVERSE
void Group::traverse(Visitor& visitor)
{
    std::for_each(_children.begin(), _children.end(), [&visitor](ref_ptr<Node>& child)
    {
        child->accept(visitor);
    });
}
#endif