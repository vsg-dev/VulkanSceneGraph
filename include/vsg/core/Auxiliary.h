#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>
#include <vsg/core/ref_ptr.h>

#include <map>
#include <mutex>

namespace vsg
{

    /** Auxiliary provides extra Object data that is rarely used, and hooks for observers.*/
    class VSG_DECLSPEC Auxiliary
    {
    public:
        std::mutex& getMutex() const { return _mutex; }

        Object* getConnectedObject() { return _connectedObject; }
        const Object* getConnectedObject() const { return _connectedObject; }

        virtual std::size_t getSizeOf() const { return sizeof(Auxiliary); }

        void ref() const;
        void unref() const;
        void unref_nodelete() const;
        inline unsigned int referenceCount() const { return _referenceCount.load(); }

        void setObject(const std::string& key, Object* object);
        Object* getObject(const std::string& key);
        const Object* getObject(const std::string& key) const;

        using ObjectMap = std::map<std::string, vsg::ref_ptr<Object>>;
        ObjectMap& getObjectMap() { return _objectMap; }
        const ObjectMap& getObjectMap() const { return _objectMap; }

        void setAllocator(Allocator* allocator) { _allocator = allocator; }
        Allocator* getAllocator() { return _allocator; }

    protected:
        explicit Auxiliary(Allocator* allocator);
        explicit Auxiliary(Object* object, Allocator* allocator = nullptr);

        virtual ~Auxiliary();

        /// reset the ConnectedObject pointer to 0 unless the ConnectedObject referenceCount goes back above 0,
        /// return true if ConnectedObject should still be deleted, or false if the object should be kept.
        bool signalConnectedObjectToBeDeleted();

        void resetConnectedObject();

        friend class Object;
        friend class Allocator;

        mutable std::atomic_uint _referenceCount;

        mutable std::mutex _mutex;
        Object* _connectedObject;

        ref_ptr<Allocator> _allocator;
        ObjectMap _objectMap;
    };

} // namespace vsg
