#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Data.h>

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>

#include <vsg/io/Input.h>
#include <vsg/io/Output.h>

#define VSG_array2D(N, T) \
    using N = Array2D<T>; \
    template<>            \
    constexpr const char* type_name<N>() noexcept { return "vsg::" #N; }

namespace vsg
{
    template<typename T>
    class Array2D : public Data
    {
    public:
        using value_type = T;
        using iterator = value_type*;
        using const_iterator = const value_type*;

        Array2D() :
            _width(0),
            _height(),
            _data(nullptr) {}
        Array2D(std::size_t width, std::size_t height, value_type* data) :
            _width(width),
            _height(height),
            _data(data) {}
        explicit Array2D(std::size_t width, std::size_t height) :
            _width(width),
            _height(height),
            _data(new value_type[width * height]) {}

        std::size_t sizeofObject() const noexcept override { return sizeof(Array2D); }

        // implementation provided by Visitor.h
        void accept(Visitor& visitor) override;
        void accept(ConstVisitor& visitor) const override;

        const char* className() const noexcept override { return type_name<Array2D>(); }

        void read(Input& input) override
        {
            Data::read(input);
            size_t width = input.readValue<uint32_t>("Width");
            size_t height = input.readValue<uint32_t>("Height");
            size_t new_size = width * height;
            if (input.matchPropertyName("Data"))
            {
                if (_data) // if data already may be able to reuse it
                {
                    size_t original_size = width * height;
                    if (original_size != new_size) // if existing data is a different size delete old, and create new
                    {
                        delete[] _data;
                        _data = new value_type[new_size];
                    }
                }
                else // allocate space for data
                {
                    _data = new value_type[new_size];
                }
                _width = width;
                _height = height;
                input.read(new_size, _data);
            }
        }

        void write(Output& output) const override
        {
            Data::write(output);
            output.writeValue<uint32_t>("Width", _width);
            output.writeValue<uint32_t>("Height", _height);
            output.writePropertyName("Data");
            output.write(_width * _height, _data);
        }

        std::size_t size() const { return _width * _height; }
        bool empty() const { return _width == 0 && _height == 0; }

        void clear()
        {
            _width = 0;
            _height = 0;
            if (_data) { delete[] _data; }
            _data = nullptr;
        }

        void assign(std::size_t width, std::size_t height, value_type* data)
        {
            if (_data) delete[] _data;

            _width = width;
            _height = height;
            _data = data;
        }

        // release the data so that owneership can be passed on, the local data pointer and size is set to 0 and destruction of Array will no result in the data being deleted.
        void* dataRelease() override
        {
            void* tmp = _data;
            _data = nullptr;
            _width = 0;
            _height = 0;
            return tmp;
        }

        std::size_t valueSize() const override { return sizeof(value_type); }
        std::size_t valueCount() const override { return _width * _height; }

        std::size_t dataSize() const override { return (_width * _height) * sizeof(value_type); }
        void* dataPointer() override { return _data; }
        const void* dataPointer() const override { return _data; }

        size_t width() const { return _width; }
        size_t height() const { return _height; }

        value_type* data() { return _data; }
        const value_type* data() const { return _data; }

        value_type& operator[](std::size_t i) { return _data[i]; }
        const value_type& operator[](std::size_t i) const { return _data[i]; }

        value_type& at(std::size_t i) { return _data[i]; }
        const value_type& at(std::size_t i) const { return _data[i]; }

        value_type& operator()(std::size_t i, std::size_t j) { return _data[i + j * _width]; }
        const value_type& operator()(std::size_t i, std::size_t j) const { return _data[i + j * _width]; }

        value_type& at(std::size_t i, std::size_t j) { return _data[i + j * _width]; }
        const value_type& at(std::size_t i, std::size_t j) const { return _data[i + j * _width]; }

        void set(std::size_t i, const value_type& v) { _data[i] = v; }
        void set(std::size_t i, std::size_t j, const value_type& v) { _data[i + j * _width] = v; }

        iterator begin() { return _data; }
        const_iterator begin() const { return _data; }

        iterator end() { return _data + (_width * _height); }
        const_iterator end() const { return _data + (_width * _height); }

    protected:
        virtual ~Array2D()
        {
            if (_data) delete[] _data;
        }

    private:
        std::size_t _width;
        std::size_t _height;
        value_type* _data;
    };

    VSG_array2D(ubyteArray2D, std::uint8_t);
    VSG_array2D(ushortArray2D, std::uint16_t);
    VSG_array2D(uintArray2D, std::uint32_t);
    VSG_array2D(floatArray2D, float);
    VSG_array2D(doubleArray2D, double);

    VSG_array2D(vec2Array2D, vec2);
    VSG_array2D(vec3Array2D, vec3);
    VSG_array2D(vec4Array2D, vec4);
    VSG_array2D(mat4Array2D, mat4);

    VSG_array2D(dvec2Array2D, dvec2);
    VSG_array2D(dvec3Array2D, dvec3);
    VSG_array2D(dvec4Array2D, dvec4);
    VSG_array2D(dmat4Array2D, dmat4);

} // namespace vsg
