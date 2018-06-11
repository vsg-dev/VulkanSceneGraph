#pragma once

#include <vsg/core/Visitor.h>

#include <atomic>
#include <string>


namespace vsg
{

    // forward declare
    class Auxiliary;

    class Object
    {
    public:
        Object();

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        void ref() const;
        void unref() const;
        void unref_nodelete() const;
        inline unsigned int referenceCount() const { return _referenceCount.load(); }


        struct Key
        {
            Key(const char* in_name) : name(in_name), index(0) {}
            Key(const std::string& in_name) : name(in_name), index(0) {}
            Key(int in_index) : index(in_index) {}

            Key(const char* in_name, int in_index) : name(in_name), index(in_index) {}
            Key(const std::string& in_name, int in_index) : name(in_name), index(in_index) {}

            bool operator < (const Key& rhs) const
            {
                if (name<rhs.name) return true;
                if (name>rhs.name) return false;
                return (index<rhs.index);
            }

            std::string name;
            int         index;
        };

        template<typename T>
        void setValue(const Key& key, const T& value);

        template<typename T>
        bool getValue(const Key& key, T& value) const;

        void setObject(const Key& key, Object* object);
        Object* getObject(const Key& key);
        const Object* getObject(const Key& key) const;

        Auxiliary* getOrCreateAuxiliary();
        Auxiliary* getAuxiliary() { return _auxiliary; }
        const Auxiliary* getAuxiliary() const { return _auxiliary; }

    protected:
        virtual ~Object();

        mutable std::atomic_uint _referenceCount;

        Auxiliary* _auxiliary;
    };

    template<typename T>
    class ValueObject : public Object
    {
    public:
        ValueObject() {}
        ValueObject(const T& in_value) : value(in_value) {}
        ValueObject(const ValueObject& rhs) : value(rhs.value) {}

        T value;

    protected:
        virtual ~ValueObject() {}
    };

    template<typename T>
    void Object::setValue(const Key& key, const T& value)
    {
        using ValueObjectT = ValueObject<T>;
        setObject(key, new ValueObjectT(value));
    }

    template<typename T>
    bool Object::getValue(const Key& key, T& value) const
    {
        using ValueObjectT = ValueObject<T>;
        const ValueObjectT* vo = dynamic_cast<const ValueObjectT*>(getObject(key));
        if (vo)
        {
            value = vo->value;
            return true;
        }
        else
        {
            return false;
        }
    }

}
