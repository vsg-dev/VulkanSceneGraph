/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Auxiliary.h>
#include <vsg/core/ConstVisitor.h>
#include <vsg/core/Object.h>
#include <vsg/core/Visitor.h>

#include <vsg/traversals/RecordTraversal.h>

#include <vsg/io/Input.h>
#include <vsg/io/Options.h>
#include <vsg/io/Output.h>

using namespace vsg;

#if 1
#    include <iostream>
#    define DEBUG_NOTIFY \
        if (false) std::cout
#else
#    include <iostream>
#    define DEBUG_NOTIFY std::cout
#endif

Object::Object() :
    _referenceCount(0),
    _auxiliary(nullptr)
{
}

Object::Object(const Object& rhs) :
    Object()
{
    if (rhs._auxiliary && rhs._auxiliary->getConnectedObject() == &rhs)
    {
        // the rhs's rhs._auxiliary is uniquely attached to it, so we need to create our own and copy it's ObjectMap across
        Auxiliary::ObjectMap& objectMap = getOrCreateUniqueAuxiliary()->getObjectMap();
        objectMap = rhs._auxiliary->getObjectMap();
    }
}

Object& Object::operator=(const Object& rhs)
{
    // std::cout << "Object& operator=(const Object&)" << std::endl;
    if (&rhs == this) return *this;

    if (rhs._auxiliary && rhs._auxiliary->getConnectedObject() == &rhs)
    {
        // the rhs's rhs._auxiliary is uniquely attached to it, so we need to create our own and copy it's ObjectMap across
        Auxiliary::ObjectMap& objectMap = getOrCreateUniqueAuxiliary()->getObjectMap();
        objectMap = rhs._auxiliary->getObjectMap();
    }

    return *this;
}

Object::Object(Allocator* allocator) :
    _referenceCount(0),
    _auxiliary(nullptr)
{
    if (allocator) _auxiliary = allocator->getOrCreateSharedAuxiliary();
}

Object::~Object()
{
    DEBUG_NOTIFY << "Object::~Object() " << this << std::endl;

    if (_auxiliary)
    {
        _auxiliary->unref();
    }
}

void Object::_attemptDelete() const
{
    // what should happen when _delete is called on an Object with ref() of zero?  Need to decide whether this buggy application usage should be tested for.

    // if there is an auxiliary attached signal to it we wish to delete, and give it an opportunity to decide whether a delete is appropriate.
    // if no auxiliary is attached then go straight ahead and delete.
    if (_auxiliary == nullptr || _auxiliary->signalConnectedObjectToBeDeleted())
    {
        DEBUG_NOTIFY << "Object::_delete() " << this << " calling delete" << std::endl;

        ref_ptr<Allocator> allocator(getAllocator());
        if (allocator)
        {
            std::size_t size = sizeofObject();

            DEBUG_NOTIFY << "Calling this->~Object();" << std::endl;
            this->~Object();

            DEBUG_NOTIFY << "After Calling this->~Object();" << std::endl;
            allocator->deallocate(this, size);
        }
        else
        {
            delete this;
        }
    }
    else
    {
        //std::cout<<"Object::_delete() "<<this<<" choosing not to delete"<<std::endl;
    }
}

#if 0
ref_ptr<Object> Object::create(Allocator* allocator)
{
    if (allocator)
    {
        // need to think about alignment...
        const std::size_t size = sizeof(Object);
        void* ptr = allocator->allocate(size);

        ref_ptr<Object> object(new (ptr) Object());
        object->setAuxiliary(allocator->getOrCreateSharedAuxiliary());

        // check the sizeof(Object) is consistent with Object::sizeOfObject()
        if (std::size_t new_size = object->sizeofObject(); new_size != size)
        {
            throw make_string("Warning: Allocator::create(",typeid(Object).name(),") mismatch sizeof() = ",size,", ",new_size);
        }
        return object;
    }
    else return ref_ptr<Object>(new Object());
}
#endif

void Object::accept(Visitor& visitor)
{
    visitor.apply(*this);
}

void Object::accept(ConstVisitor& visitor) const
{
    visitor.apply(*this);
}

void Object::accept(RecordTraversal& visitor) const
{
    visitor.apply(*this);
}

void Object::read(Input& input)
{
    auto numObjects = input.readValue<uint32_t>("NumUserObjects");
    if (numObjects > 0)
    {
        Auxiliary::ObjectMap& objectMap = getOrCreateUniqueAuxiliary()->getObjectMap();
        for (; numObjects > 0; --numObjects)
        {
            std::string key = input.readValue<std::string>("Key");
            input.readObject("Object", objectMap[key]);
        }
    }
}

void Object::write(Output& output) const
{
    if (_auxiliary && _auxiliary->getConnectedObject() == this)
    {
        // we have a unique auxiliary, need to write out it's ObjectMap entries
        const Auxiliary::ObjectMap& objectMap = _auxiliary->getObjectMap();
        output.writeValue<uint32_t>("NumUserObjects", objectMap.size());
        for (auto& entry : objectMap)
        {
            output.write("Key", entry.first);
            output.writeObject("Object", entry.second.get());
        }
    }
    else
    {
        output.writeValue<uint32_t>("NumUserObjects", 0);
    }
}

void Object::setObject(const std::string& key, Object* object)
{
    getOrCreateUniqueAuxiliary()->setObject(key, object);
}

Object* Object::getObject(const std::string& key)
{
    if (!_auxiliary) return nullptr;
    return _auxiliary->getObject(key);
}

const Object* Object::getObject(const std::string& key) const
{
    if (!_auxiliary) return nullptr;
    return _auxiliary->getObject(key);
}

void Object::removeObject(const std::string& key)
{
    if (_auxiliary)
    {
        _auxiliary->getObjectMap().erase(key);
    }
}

void Object::setAuxiliary(Auxiliary* auxiliary)
{
    if (_auxiliary)
    {
        _auxiliary->resetConnectedObject();
        _auxiliary->unref();
    }

    _auxiliary = auxiliary;

    if (auxiliary)
    {
        auxiliary->ref();
    }
}

Auxiliary* Object::getOrCreateUniqueAuxiliary()
{
    DEBUG_NOTIFY << "Object::getOrCreateUniqueAuxiliary() _auxiliary=" << _auxiliary << std::endl;
    if (!_auxiliary)
    {
        _auxiliary = new Auxiliary(this);
        _auxiliary->ref();
    }
    else
    {
        if (_auxiliary->getConnectedObject() != this)
        {
            Auxiliary* previousAuxiliary = _auxiliary;
            Allocator* allocator = previousAuxiliary->getAllocator();
            if (allocator)
            {
                void* ptr = allocator->allocate(sizeof(Auxiliary));
                _auxiliary = new (ptr) Auxiliary(this, allocator);
                DEBUG_NOTIFY << "   used Allocator to allocate _auxiliary=" << _auxiliary << std::endl;
            }
            else
            {
                _auxiliary = new Auxiliary(this, allocator);
            }

            _auxiliary->ref();

            DEBUG_NOTIFY << "Object::getOrCreateUniqueAuxiliary() _auxiliary=" << _auxiliary << " replaces previousAuxiliary=" << previousAuxiliary << std::endl;

            previousAuxiliary->unref();
        }
    }
    return _auxiliary;
}

Allocator* Object::getAllocator() const
{
    return _auxiliary ? _auxiliary->getAllocator() : 0;
}
