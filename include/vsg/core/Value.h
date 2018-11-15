#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Data.h>
#include <vsg/core/type_name.h>

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>

#include <vsg/io/Input.h>
#include <vsg/io/Output.h>

namespace vsg
{
    template<typename T>
    class Value : public Data
    {
    public:
        using value_type = T;

        Value() {}
        Value(const Value& rhs) :
            _value(rhs._value) {}
        explicit Value(const value_type& in_value) :
            _value(in_value) {}

        std::size_t sizeofObject() const noexcept override { return sizeof(Value); }

        // implementation provided by Visitor.h
        void accept(Visitor& visitor) override;
        void accept(ConstVisitor& visitor) const override;

        const char* className() const noexcept override { return type_name<Value>(); }

        void read(Input& input) override
        {
            Data::read(input);
            input.read("Value", _value);
        }

        void write(Output& output) const override
        {
            Data::write(output);
            output.write("Value", _value);
        }

        std::size_t valueSize() const override { return sizeof(value_type); }
        std::size_t valueCount() const override { return 1; }

        std::size_t dataSize() const override { return sizeof(value_type); }
        void* dataPointer() override { return &_value; }
        const void* dataPointer() const override { return &_value; }
        void* dataRelease() override { return nullptr; }

        Value& operator=(const Value& rhs)
        {
            _value = rhs._value;
            return *this;
        }
        Value& operator=(const value_type& rhs)
        {
            _value = rhs;
            return *this;
        }

        operator value_type&() { return _value; }
        operator const value_type&() const { return _value; }

        value_type& value() { return _value; }
        const value_type& value() const { return _value; }

    protected:
        virtual ~Value() {}

    private:
        value_type _value;
    };

    template<typename T>
    void Object::setValue(const std::string& key, const T& value)
    {
        using ValueT = Value<T>;
        setObject(key, new ValueT(value));
    }

    template<typename T>
    bool Object::getValue(const std::string& key, T& value) const
    {
        using ValueT = Value<T>;
        const Object* object = getObject(key);
        if (object && (typeid(*object) == typeid(ValueT)))
        {
            const ValueT* vo = static_cast<const ValueT*>(getObject(key));
            value = *vo;
            return true;
        }
        else
        {
            return false;
        }
    }

    using stringValue = Value<std::string>;
    using boolValue = Value<bool>;
    using intValue = Value<int>;
    using uintValue = Value<unsigned int>;
    using floatValue = Value<float>;
    using doubleValue = Value<double>;
    using vec2Value = Value<vec2>;
    using vec3Value = Value<vec3>;
    using vec4Value = Value<vec4>;
    using mat4Value = Value<mat4>;
    using dvec2Value = Value<dvec2>;
    using dvec3Value = Value<dvec3>;
    using dvec4Value = Value<dvec4>;
    using dmat4Value = Value<dmat4>;

    VSG_type_name(vsg::stringValue);
    VSG_type_name(vsg::doubleValue);

} // namespace vsg
