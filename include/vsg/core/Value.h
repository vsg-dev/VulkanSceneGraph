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
        Value(const value_type& in_value) : _value(in_value) {}
        Value(const Value& rhs) : _value(rhs._value) {}

        // implementation provided by Visitor.h
        virtual void accept(Visitor& visitor);

        virtual std::size_t valueSize() const { return sizeof(value_type); }
        virtual std::size_t valueCount() const { return 1; }

        virtual std::size_t dataSize() const { return sizeof(value_type); }
        virtual void* dataPointer() { return &_value; }
        virtual const void* dataPointer() const { return &_value; }

        Value& operator = (const Value& rhs) { _value = rhs._value; return *this; }
        Value& operator = (const value_type& rhs) { _value = rhs; return *this; }

        operator value_type& () { return _value; }
        operator const value_type& () const { return _value; }

        value_type& value() { return _value; }
        const value_type& value() const { return _value; }

    protected:
        virtual ~Value() {}
        value_type _value;
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
            value = *vo;
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
