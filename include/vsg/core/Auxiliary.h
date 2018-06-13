#pragma once

#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>

#include <map>
#include <algorithm>

namespace vsg
{

    /** Auxiliary provides extra Object data that is rarely used, and hooks for observers.*/
    class Auxiliary
    {
    public:
        Auxiliary();

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

        void traverse(Visitor& visitor)
        {
            std::for_each(_objectMap.begin(), _objectMap.end(), [&visitor](ObjectMap::value_type& value)
            {
                value.second->accept(visitor);
            });
        }

    protected:

        virtual ~Auxiliary();

        void setConnectedObject(Object* object);

        friend class Object;

        Object* _connectedObject;

        mutable std::atomic_uint _referenceCount;

        ObjectMap _objectMap;

    };

}
