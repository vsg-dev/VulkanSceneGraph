#include "ObjectConnections.hpp"

#include <iostream>

using namespace vsg;

ObjectConnections::ObjectConnections() :
    _referenceCount(0),
    _connectedObject(0)
{
    std::cout<<"ObjectConnections::ObjectConnections() "<<this<<std::endl;
}

ObjectConnections::~ObjectConnections()
{
    std::cout<<"ObjectConnections::~ObjectConnections() "<<this<<std::endl;
}

void ObjectConnections::ref() const
{
    ++_referenceCount;
    std::cout<<"Object::ref() "<<this<<" "<<_referenceCount.load()<<std::endl;
}

void ObjectConnections::unref() const
{
    std::cout<<"Object::unref() "<<this<<" "<<_referenceCount.load()<<std::endl;
    if (_referenceCount.fetch_sub(1)<=1)
    {
        delete this;
    }
}

void ObjectConnections::unref_nodelete() const
{
    std::cout<<"ObjectConnections::unref_nodelete() "<<this<<" "<<_referenceCount.load()<<std::endl;
    --_referenceCount;
}


void ObjectConnections::setConnectedObject(Object* object)
{
    std::cout<<"ObjectConnections::setConnectedObject("<<object<<") previous _connectedObject="<<_connectedObject<<std::endl;
    _connectedObject = object;
}

void ObjectConnections::setObject(const Object::Key& key, Object* object)
{
    std::cout<<"ObjectConnections::setObject( ["<<key.name<<", "<<key.index<<"], "<<object<<")"<<std::endl;
    _objectMap[key] = object;
}

Object* ObjectConnections::getObject(const Object::Key& key)
{
    std::cout<<"ObjectConnections::getObject( ["<<key.name<<", "<<key.index<<"])"<<std::endl;
    ObjectMap::iterator itr = _objectMap.find(key);
    if (itr != _objectMap.end()) return itr->second.get();
    else return nullptr;
}

const Object* ObjectConnections::getObject(const Object::Key& key) const
{
    std::cout<<"ObjectConnections::getObject( ["<<key.name<<", "<<key.index<<"]) const"<<std::endl;
    ObjectMap::const_iterator itr = _objectMap.find(key);
    if (itr != _objectMap.end()) return itr->second.get();
    else return nullptr;
}

