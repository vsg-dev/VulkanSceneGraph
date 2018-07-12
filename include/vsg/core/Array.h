#pragma once

#include <vsg/core/Object.h>

#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>
#include <vsg/maths/mat4.h>

namespace vsg
{
    template<typename T>
    class Array : public Object
    {
    public:
        using value_type = T;
        using iterator = value_type*;
        using const_iterator = const value_type*;

        Array() : _size(0), _data(nullptr) {}
        Array(std::size_t numElements) : _size(numElements), _data(new value_type[numElements]) {}
        Array(const Array& rhs) : _data(rhs.data) {}

        // implementation provided by Visitor.h
        virtual void accept(Visitor& visitor);

        std::size_t size() const { return _size; }
        bool empty() const { return _size==0; }

        // should Array be fixed size?
        void clear() { _size = 0; if (_data) { delete [] _data; } _data = nullptr; }
        void resize(std::size_t size) { _size = size; _data = size>0 ? new value_type[size] : nullptr; }

        std::size_t dataSize() const { return _size * sizeof(value_type); }

        value_type* data() { return _data; }
        const value_type* data() const { return _data; }

        value_type& operator [] (std::size_t i) { return _data[i]; }
        const value_type& operator [] (std::size_t i) const { return _data[i]; }

        value_type& at(std::size_t i) { return _data[i]; }
        const value_type& at(std::size_t i) const { return _data[i]; }

        void set(std::size_t i, const value_type& v) { _data[i] = v; }

        iterator begin() { return _data; }
        const const_iterator begin() const { return _data; }

        iterator end() { return _data+_size; }
        const_iterator end() const { return _data+_size; }

    protected:
        virtual ~Array() { if (_data) delete [] _data; }

        std::size_t _size;
        value_type* _data;
    };

    using uintArray = Array<std::uint32_t>;
    using floatArray = Array<float>;
    using doubleArray = Array<double>;

    using vec2Array = Array<vec2>;
    using vec3Array = Array<vec3>;
    using vec4Array = Array<vec4>;
    using mat4Array = Array<mat4>;

    using dvec2Array = Array<dvec2>;
    using dvec3Array = Array<dvec3>;
    using dvec4Array = Array<dvec4>;
    using dmat4Array = Array<dmat4>;
}
