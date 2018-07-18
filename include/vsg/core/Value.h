#pragma once

#include <vsg/core/Data.h>

#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>
#include <vsg/maths/mat4.h>

namespace vsg
{
    template<typename T>
    class Value : public Data
    {
    public:
        using value_type = T;

        Value() {}
        Value(const value_type& in_value) : value(in_value) {}
        Value(const Value& rhs) : value(rhs.value) {}

        // implementation provided by Visitor.h
        virtual void accept(Visitor& visitor);

        virtual std::size_t dataSize() const { return sizeof(value_type); }
        virtual void* dataPointer() { &value; }
        virtual const void* dataPointer() const { return &value; }

        Value& operator = (const Value& rhs) { value = rhs.value; return *this; }
        Value& operator = (const value_type& rhs) { value = rhs; return *this; }

        operator value_type& () { return value; }
        operator const value_type& () const { return value; }

        value_type value;

    protected:
        virtual ~Value() {}
    };

    template<typename T>
    void Object::setValue(const Key& key, const T& value)
    {
        using ValueT = Value<T>;
        setObject(key, new ValueT(value));
    }

    template<typename T>
    bool Object::getValue(const Key& key, T& value) const
    {
        using ValueT = Value<T>;
        const ValueT* vo = dynamic_cast<const ValueT*>(getObject(key));
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

    using StringValue = Value<std::string>;
    using IntValue = Value<int>;
    using UIntValue = Value<unsigned int>;
    using FloatValue = Value<float>;
    using DoubleValue = Value<double>;
    using vec2Value = Value<vec2>;
    using vec3Value = Value<vec3>;
    using vec4Value = Value<vec4>;
    using mat4Value = Value<mat4>;
    using dvec2Value = Value<dvec2>;
    using dvec3Value = Value<dvec3>;
    using dvec4Value = Value<dvec4>;
    using dmat4Value = Value<dmat4>;
}
