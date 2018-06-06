#include "Node.hpp"

#include <iostream>
#include <vector>

using namespace vsg;


Node::Node()
{
    std::cout<<"Node::Node() "<<this<<std::endl;
}

Node::~Node()
{
    std::cout<<"Node::~Node() "<<this<<std::endl;
}
