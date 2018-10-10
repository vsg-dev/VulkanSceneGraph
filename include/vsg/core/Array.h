#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Data.h>

#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>
#include <vsg/maths/mat4.h>

namespace vsg
{
    template<typename T>
    class Array : public Data
    {
    public:
        using value_type = T;
        using iterator = value_type*;
        using const_iterator = const value_type*;

        Array() : _size(0), _data(nullptr) {}
        Array(std::size_t numElements) : _size(numElements), _data(new value_type[numElements]) {}
        Array(std::size_t numElements, value_type* data) : _size(numElements), _data(data) {}
        Array(std::initializer_list<value_type> l) : _size(l.size()), _data(new value_type[l.size()]) { value_type* ptr = _data; for (value_type const & v : l) { (*ptr++) = v; } }
        Array(const Array& rhs) : _data(rhs.data) {}

        std::size_t getSizeOf() const noexcept override { return sizeof(Data); }

        // implementation provided by Visitor.h
        void accept(Visitor& visitor) override ;

        std::size_t size() const { return _size; }
        bool empty() const { return _size==0; }

        // should Array be fixed size?
        void clear() { _size = 0; if (_data) { delete [] _data; } _data = nullptr; }

        void assign(std::size_t numElements, value_type* data)
        {
            if (_data!=nullptr) delete [] _data;

            _size = numElements;
            _data = data;
        }

        void resize(std::size_t size)
        {
            if (_data)
            {
                value_type* original_data = _data;

                std::size_t size_to_copy = std::min(_size, size);

                _size = size;
                _data = size>0 ? new value_type[size] : nullptr;

                // copy data
                for(std::size_t i=0; i<size_to_copy; ++i) _data[i] = original_data[i];

                delete [] original_data;
            }
            else
            {
                _size = size;
                _data = size>0 ? new value_type[size] : nullptr;
            }
        }

        // release the data so that owneership can be passed on, the local data pointer and size is set to 0 and destruction of Array will no result in the data being deleted.
        void* dataRelease() override { void* tmp = _data; _data = nullptr; _size=0; return tmp; }

        std::size_t valueSize() const override { return sizeof(value_type); }
        std::size_t valueCount() const override { return _size; }

        std::size_t dataSize() const override { return _size * sizeof(value_type); }
        void* dataPointer() override { return _data; }
        const void* dataPointer() const override { return _data; }

        value_type* data() { return _data; }
        const value_type* data() const { return _data; }

        value_type& operator [] (std::size_t i) { return _data[i]; }
        const value_type& operator [] (std::size_t i) const { return _data[i]; }

        value_type& at(std::size_t i) { return _data[i]; }
        const value_type& at(std::size_t i) const { return _data[i]; }

        void set(std::size_t i, const value_type& v) { _data[i] = v; }

        iterator begin() { return _data; }
        const_iterator begin() const { return _data; }

        iterator end() { return _data+_size; }
        const_iterator end() const { return _data+_size; }

    protected:
        virtual ~Array() { if (_data) delete [] _data; }

    private:
        std::size_t _size;
        value_type* _data;
    };

    using ubyteArray = Array<std::uint8_t>;
    using ushortArray = Array<std::uint16_t>;
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
