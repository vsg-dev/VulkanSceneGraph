/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Auxiliary.h>
#include <vsg/io/Input.h>
#include <vsg/io/Options.h>
#include <vsg/io/Output.h>

#if 1
#    include <iostream>
#    define DEBUG_NOTIFY \
        if (false) std::cout
#else
#    include <iostream>
#    define DEBUG_NOTIFY std::cout
#endif

using namespace vsg;

Auxiliary::Auxiliary(Object* object) :
    _referenceCount(0),
    _connectedObject(object)
{
    DEBUG_NOTIFY << "Auxiliary::Auxiliary(Object = " << object << ") " << this << " " << std::endl;
}

Auxiliary::~Auxiliary()
{
    DEBUG_NOTIFY << "Auxiliary::~Auxiliary() " << this << std::endl;
}

void Auxiliary::ref() const
{
    ++_referenceCount;
    DEBUG_NOTIFY << "Auxiliary::ref() " << this << " " << _referenceCount.load() << std::endl;
}

void Auxiliary::unref() const
{
    DEBUG_NOTIFY << "Auxiliary::unref() " << this << " " << _referenceCount.load() << std::endl;
    if (_referenceCount.fetch_sub(1) <= 1)
    {
        delete this;
    }
}

void Auxiliary::unref_nodelete() const
{
    //std::cout<<"Auxiliary::unref_nodelete() "<<this<<" "<<_referenceCount.load()<<std::endl;
    --_referenceCount;
}

bool Auxiliary::signalConnectedObjectToBeDeleted()
{
    std::scoped_lock<std::mutex> guard(_mutex);

    if (_connectedObject && _connectedObject->referenceCount() > 0)
    {
        // return false, the object should not be deleted
        return false;
    }

    // disconnect this Auxiliary object from the ConnectedObject
    _connectedObject = 0;

    // return true, the object should be deleted
    return true;
}

void Auxiliary::resetConnectedObject()
{
    std::scoped_lock<std::mutex> guard(_mutex);

    _connectedObject = 0;
}

void Auxiliary::setObject(const std::string& key, Object* object)
{
    userObjects[key] = object;
    DEBUG_NOTIFY << "Auxiliary::setObject( [" << key << "], " << object << ")"
                 << " " << userObjects.size() << " " << &userObjects << std::endl;
}

Object* Auxiliary::getObject(const std::string& key)
{
    DEBUG_NOTIFY << "Auxiliary::getObject( [" << key << "])" << std::endl;
    if (auto itr = userObjects.find(key); itr != userObjects.end())
        return itr->second.get();
    else
        return nullptr;
}

const Object* Auxiliary::getObject(const std::string& key) const
{
    DEBUG_NOTIFY << "Auxiliary::getObject( [" << key << "]) const" << std::endl;
    if (auto itr = userObjects.find(key); itr != userObjects.end())
        return itr->second.get();
    else
        return nullptr;
}
