/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Object.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/Auxiliary.h>

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

void Object::ref() const
{
    ++_referenceCount;
}

void Object::unref() const
{
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
    --_referenceCount;
}

void Object::accept(Visitor& visitor)
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


