/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Object.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/Auxiliary.h>

#include <vsg/traversals/DispatchTraversal.h>

#include <iostream>

using namespace vsg;

Object::Object() :
    _referenceCount(0),
    _auxiliary(nullptr)
{
}

Object::~Object()
{
    if (_auxiliary)
    {
        _auxiliary->setConnectedObject(0);
        _auxiliary->unref();
    }
}

void Object::_delete() const
{
    // what should happen when _delete is called on an Object with ref() of zero?  Need to decide whether this buggy application usage should be tested for.

    // if there is an auxiliary attached signal to it we wish to delete, and give it an opportunity to decide whether a delete is appropriate.
    // if no auxiliary is attached then go straight ahead and delete.
    if (_auxiliary==nullptr || _auxiliary->signalConnectedObjectToBeDeleted())
    {
        //std::cout<<"Object::_delete() "<<this<<" calling delete"<<std::endl;

        delete this;
    }
    else
    {
        //std::cout<<"Object::_delete() "<<this<<" choosing not to delete"<<std::endl;
    }
}

void Object::accept(Visitor& visitor)
{
    visitor.apply(*this);
}

void Object::accept(DispatchTraversal& visitor) const
{
    visitor.apply(*this);
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


