#include <vsg/nodes/Node.h>

#include <iostream>

using namespace vsg;

Node::Node()
{
    std::cout<<"Node::Node()"<<std::endl;
}

Node::~Node()
{
    std::cout<<"Node::~Node()"<<std::endl;
}
