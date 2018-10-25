/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Auxiliary.h>

#if 1
#include <iostream>
#define DEBUG_NOTIFY if (false) std::cout
#else
#include <iostream>
#define DEBUG_NOTIFY std::cout
#endif

using namespace vsg;

Auxiliary::Auxiliary(Allocator* allocator) :
    _referenceCount(0),
    _connectedObject(0),
    _allocator(allocator)
{
    DEBUG_NOTIFY<<"Auxiliary::Auxiliary(Allocator = "<<allocator<<") "<<this<<" "<<std::endl;
}

Auxiliary::Auxiliary(Object* object, Allocator* allocator) :
    _referenceCount(0),
    _connectedObject(object),
    _allocator(allocator)
{
    DEBUG_NOTIFY<<"Auxiliary::Auxiliary(Object = "<<object<<", Allocator = "<<allocator<<") "<<this<<" "<<std::endl;
}

Auxiliary::~Auxiliary()
{
    DEBUG_NOTIFY<<"Auxiliary::~Auxiliary() "<<this<<std::endl;
    if (_allocator) _allocator->detachSharedAuxiliary(this);
}

void Auxiliary::ref() const
{
    ++_referenceCount;
    DEBUG_NOTIFY<<"Auxiliary::ref() "<<this<<" "<<_referenceCount.load()<<std::endl;
}

void Auxiliary::unref() const
{
    DEBUG_NOTIFY<<"Auxiliary::unref() "<<this<<" "<<_referenceCount.load()<<std::endl;
    if (_referenceCount.fetch_sub(1)<=1)
    {
        if (_allocator)
        {
            ref_ptr<Allocator> allocator = _allocator;

            std::size_t size = getSizeOf();

            DEBUG_NOTIFY<<"  Calling this->~Auxiliary();"<<std::endl;
            this->~Auxiliary();

            DEBUG_NOTIFY<<"  After Calling this->~Auxiliary();"<<std::endl;
            allocator->deallocate(this, size);
        }
        else
        {
            delete this;
        }
    }
}

void Auxiliary::unref_nodelete() const
{
    //std::cout<<"Auxiliary::unref_nodelete() "<<this<<" "<<_referenceCount.load()<<std::endl;
    --_referenceCount;
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

void Auxiliary::resetConnectedObject()
{
    _connectedObject.exchange(0);
}


void Auxiliary::setObject(const std::string& key, Object* object)
{
    _objectMap[key] = object;
    DEBUG_NOTIFY<<"Auxiliary::setObject( ["<<key<<"], "<<object<<")"<<" "<<_objectMap.size()<<" "<<&_objectMap<<std::endl;
}

Object* Auxiliary::getObject(const std::string& key)
{
    DEBUG_NOTIFY<<"Auxiliary::getObject( ["<<key<<"])"<<std::endl;
    ObjectMap::iterator itr = _objectMap.find(key);
    if (itr != _objectMap.end()) return itr->second.get();
    else return nullptr;
}

const Object* Auxiliary::getObject(const std::string& key) const
{
    DEBUG_NOTIFY<<"Auxiliary::getObject( ["<<key<<"]) const"<<std::endl;
    ObjectMap::const_iterator itr = _objectMap.find(key);
    if (itr != _objectMap.end()) return itr->second.get();
    else return nullptr;
}

