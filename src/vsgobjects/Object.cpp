#include "Object.h"
#include "Auxiliary.h"

#include <iostream>

using namespace vsg;

Object::Object() :
    _referenceCount(0),
    _auxiliary(nullptr)
{
    std::cout<<"Object::Object() "<<this<<std::endl;
}

Object::~Object()
{
    std::cout<<"Object::~Object() "<<this<<" "<<_referenceCount.load()<<std::endl;
    if (_auxiliary)
    {
        _auxiliary->setConnectedObject(0);
        _auxiliary->unref();
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
        // what should happen when unref() called on an Object with ref() of zero?  Need to decide whether this buggy application usage should be tested for.
        if (_auxiliary)
        {
            _auxiliary->setConnectedObject(0);

            if (_referenceCount.load()>0)
            {
                // in between the fetch_sub and the completion of setConnectedObject(0) a references was taken by another thread,
                // we restore the connection  and abort the attempt to delete as it's no longer required.
                _auxiliary->setConnectedObject(const_cast<Object*>(this));
                return;
            }
        }

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
    getOrCreateAuxiliary()->setObject(key, object);
}

Object* Object::getObject(const Key& key)
{
    if (!_auxiliary) return nullptr;
    return _auxiliary->getObject(key);
}

const Object* Object::getObject(const Key& key) const
{
    if (!_auxiliary) return nullptr;
    return _auxiliary->getObject(key);
}


Auxiliary* Object::getOrCreateAuxiliary()
{
    if (!_auxiliary)
    {
        _auxiliary = new Auxiliary;
        _auxiliary->ref();
        _auxiliary->setConnectedObject(this);
    }
    return _auxiliary;
}


