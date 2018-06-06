#pragma once

#include "Object.hpp"

#include <map>

namespace vsg
{

    class ObjectConnections
    {
    public:
        ObjectConnections();

        Object* getConnectedObject() { return _connectedObject; }

        void ref() const;
        void unref() const;
        void unref_nodelete() const;
        inline unsigned int referenceCount() const { return _referenceCount.load(); }

        void setObject(const Object::Key& key, Object* object);
        Object* getObject(const Object::Key& key);
        const Object* getObject(const Object::Key& key) const;

        typedef std::map< Object::Key, vsg::ref_ptr<Object> > ObjectMap;
        ObjectMap& getObjectMap() { _objectMap; }
        const ObjectMap& getObjectMap() const { _objectMap; }

    protected:

        virtual ~ObjectConnections();

        void setConnectedObject(Object* object);

        friend class Object;

        Object* _connectedObject;

        mutable std::atomic_uint _referenceCount;

        ObjectMap _objectMap;

    };

}