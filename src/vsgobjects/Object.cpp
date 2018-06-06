#include "Object.hpp"

#include <iostream>

using namespace vsg;

Object::Object() :
    _referenceCount(0)
{
    std::cout<<"Object::Object() "<<this<<std::endl;
}

Object::~Object()
{
    std::cout<<"Object::~Object() "<<this<<" "<<_referenceCount.load()<<std::endl;
}

void Object::ref() const
{
    ++_referenceCount;
    std::cout<<"Object::ref() "<<this<<" "<<_referenceCount.load()<<std::endl;
}

void Object::unref() const
{
    std::cout<<"Object::unref() "<<this<<" "<<_referenceCount.load()<<std::endl;
    if (_referenceCount.fetch_sub(1)<=1)
    {
        delete this;
    }
}

void Object::unref_nodelete() const
{
    std::cout<<"Object::unref_nodelete() "<<this<<" "<<_referenceCount.load()<<std::endl;
    --_referenceCount;
}

