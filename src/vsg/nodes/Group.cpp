#include <vsg/nodes/Group.h>

#include <iostream>

using namespace vsg;

Group::Group()
{
    std::cout<<"Group::Group()"<<std::endl;
}

Group::~Group()
{
    std::cout<<"Group::~Group()"<<std::endl;
}
