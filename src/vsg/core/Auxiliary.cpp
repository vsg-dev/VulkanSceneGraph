/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

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

bool Auxiliary::signalConnectedObjectToBeDeleted()
{
    Object* previousPtr = _connectedObject.exchange(0);
    if (previousPtr && previousPtr->referenceCount()>0)
    {
        // referenceCount has been incremented by another thread, so now restore the _connectedObject
        _connectedObject.exchange(previousPtr);

        // return false, the object should not be deleted
        return false;
    }

    // return true, the object should be deleted
    return true;
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

