#include <vsg/core/Auxiliary.h>

using namespace vsg;

Auxiliary::Auxiliary() :
    _referenceCount(0),
    _connectedObject(0)
{
    //std::cout<<"Auxiliary::Auxiliary() "<<this<<" "<<_objectMap.size()<<std::endl;
}

Auxiliary::~Auxiliary()
{
    //std::cout<<"Auxiliary::~Auxiliary() "<<this<<std::endl;
}

void Auxiliary::ref() const
{
    ++_referenceCount;
    //std::cout<<"Auxiliary::ref() "<<this<<" "<<_referenceCount.load()<<std::endl;
}

void Auxiliary::unref() const
{
    //std::cout<<"Auxiliary::unref() "<<this<<" "<<_referenceCount.load()<<std::endl;
    if (_referenceCount.fetch_sub(1)<=1)
    {
        delete this;
    }
}

void Auxiliary::unref_nodelete() const
{
    //std::cout<<"Auxiliary::unref_nodelete() "<<this<<" "<<_referenceCount.load()<<std::endl;
    --_referenceCount;
}


void Auxiliary::setConnectedObject(Object* object)
{
    _connectedObject = object;
    //std::cout<<"Auxiliary::setConnectedObject("<<object<<") previous _connectedObject="<<_connectedObject<<std::endl;
}

void Auxiliary::setObject(const Object::Key& key, Object* object)
{
    _objectMap[key] = object;
    //std::cout<<"Auxiliary::setObject( ["<<key.name<<", "<<key.index<<"], "<<object<<")"<<" "<<_objectMap.size()<<" "<<&_objectMap<<std::endl;
}

Object* Auxiliary::getObject(const Object::Key& key)
{
    //std::cout<<"Auxiliary::getObject( ["<<key.name<<", "<<key.index<<"])"<<std::endl;
    ObjectMap::iterator itr = _objectMap.find(key);
    if (itr != _objectMap.end()) return itr->second.get();
    else return nullptr;
}

const Object* Auxiliary::getObject(const Object::Key& key) const
{
    //std::cout<<"Auxiliary::getObject( ["<<key.name<<", "<<key.index<<"]) const"<<std::endl;
    ObjectMap::const_iterator itr = _objectMap.find(key);
    if (itr != _objectMap.end()) return itr->second.get();
    else return nullptr;
}

