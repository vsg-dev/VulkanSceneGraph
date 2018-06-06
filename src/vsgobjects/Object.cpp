#include "Object.hpp"
#include "ObjectConnections.hpp"

#include <iostream>

using namespace vsg;

Object::Object() :
    _referenceCount(0),
    _objectConnections(nullptr)
{
    std::cout<<"Object::Object() "<<this<<std::endl;
}

Object::~Object()
{
    std::cout<<"Object::~Object() "<<this<<" "<<_referenceCount.load()<<std::endl;
    if (_objectConnections)
    {
        _objectConnections->setConnectedObject(0);
        _objectConnections->unref();
    }
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

void Object::setObject(const Key& key, Object* object)
{
    getOrCreateObjectConnections()->setObject(key, object);
}

Object* Object::getObject(const Key& key)
{
    if (!_objectConnections) return nullptr;
    return _objectConnections->getObject(key);
}

const Object* Object::getObject(const Key& key) const
{
    if (!_objectConnections) return nullptr;
    return _objectConnections->getObject(key);
}


ObjectConnections* Object::getOrCreateObjectConnections()
{
    if (!_objectConnections)
    {
        _objectConnections = new ObjectConnections;
        _objectConnections->ref();
        _objectConnections->setConnectedObject(this);
    }
    return _objectConnections;
}


