#include "ref_ptr.hpp"
#include "Object.hpp"
#include "Group.hpp"

#include <iostream>
#include <vector>

vsg::Group* createGroup()
{
    vsg::ref_ptr<vsg::Group> group = new vsg::Group;
    return group.release();
}

int main(int argc, char** argv)
{
    std::cout<<"-- Before"<<std::endl;
    vsg::ref_ptr<vsg::Object> global;
    {
        std::cout<<"---- Start of block"<<std::endl;

        vsg::ref_ptr<vsg::Group> group(createGroup());

        std::cout<<"Adding child to group"<<std::endl;
        size_t pos = group->addChild(new vsg::Node);

        std::cout<<"Removing child to group"<<std::endl;
        group->removeChild(pos);

        global = group;
        std::cout<<"---- End of block"<<std::endl;

    }
    std::cout<<"-- After"<<std::endl;
    return 0;
}