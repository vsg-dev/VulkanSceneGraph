#include "Group.h"

#include <iostream>
#include <vector>

using namespace vsg;


Group::Group()
{
    std::cout<<"Group::Group() "<<this<<std::endl;
}

Group::~Group()
{
    std::cout<<"Group::~Group() "<<this<<std::endl;
}
