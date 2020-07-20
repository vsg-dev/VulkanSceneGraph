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

#define VSG_array3D(N, T) \
    using N = Array3D<T>; \
    template<>            \
    constexpr const char* type_name<N>() noexcept { return "vsg::" #N; }

namespace vsg
{
    template<typename T>
    class Array3D : public Data
    {
    public:
        using value_type = T;
        using iterator = value_type*;
        using const_iterator = const value_type*;

        Array3D() :
            _width(0),
            _height(0),
            _depth(0),
            _data(nullptr) {}

        Array3D(uint32_t width, uint32_t height, uint32_t depth, Layout layout = {}) :
            Data(layout),
            _width(width),
            _height(height),
            _depth(depth),
            _data(new value_type[static_cast<std::size_t>(width) * height * depth]) {}

        Array3D(uint32_t width, uint32_t height, uint32_t depth, value_type* data, Layout layout = {}) :
            Data(layout),
            _width(width),
            _height(height),
            _depth(depth),
            _data(data) {}

        Array3D(uint32_t width, uint32_t height, uint32_t depth, const value_type& value, Layout layout = {}) :
            Data(layout),
            _width(width),
            _height(height),
            _depth(depth),
            _data(new value_type[static_cast<std::size_t>(width) * height * depth])
        {
            for (auto& v : *this) v = value;
        }

        template<typename... Args>
        static ref_ptr<Array3D> create(Args... args)
        {
            return ref_ptr<Array3D>(new Array3D(args...));
        }

        std::size_t sizeofObject() const noexcept override { return sizeof(Array3D); }

        // implementation provided by Visitor.h
        void accept(Visitor& visitor) override;
        void accept(ConstVisitor& visitor) const override;

        const char* className() const noexcept override { return type_name<Array3D>(); }

        void read(Input& input) override
        {
            std::size_t original_size = size();

            Data::read(input);
            uint32_t width = input.readValue<uint32_t>("Width");
            uint32_t height = input.readValue<uint32_t>("Height");
            uint32_t depth = input.readValue<uint32_t>("Depth");
            std::size_t new_size = computeValueCountIncludingMipmaps(width, height, depth, _layout.maxNumMipmaps);
            if (input.matchPropertyName("Data"))
            {
                if (_data) // if data already may be able to reuse it
                {
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
                _depth = depth;

                input.read(new_size, _data);
            }
        }

        void write(Output& output) const override
        {
            Data::write(output);
            output.writeValue<uint32_t>("Width", _width);
            output.writeValue<uint32_t>("Height", _height);
            output.writeValue<uint32_t>("Depth", _depth);

            output.writePropertyName("Data");
            output.write(valueCount(), _data);
            output.writeEndOfLine();
        }

        std::size_t size() const { return (_layout.maxNumMipmaps <= 1) ? (static_cast<std::size_t>(_width) * _height * _depth) : computeValueCountIncludingMipmaps(_width, _height, _depth, _layout.maxNumMipmaps); }

        bool empty() const { return _width == 0 && _height == 0 && _depth == 0; }

        void clear()
        {
            _width = 0;
            _height = 0;
            _depth = 0;
            if (_data) { delete[] _data; }
            _data = nullptr;
        }

        void assign(uint32_t width, uint32_t height, uint32_t depth, value_type* data, Layout layout = Layout())
        {
            if (_data) delete[] _data;

            _layout = layout;
            _width = width;
            _height = height;
            _depth = depth;
            _data = data;
        }


        // release the data so that ownership can be passed on, the local data pointer and size is set to 0 and destruction of Array will no result in the data being deleted.
        void* dataRelease() override
        {
            void* tmp = _data;
            _data = nullptr;
            _width = 0;
            _height = 0;
            _depth = 0;
            return tmp;
        }

        std::size_t valueSize() const override { return sizeof(value_type); }
        std::size_t valueCount() const override { return size(); }

        std::size_t dataSize() const override { return size() * sizeof(value_type); }

        void* dataPointer() override { return _data; }
        const void* dataPointer() const override { return _data; }

        void* dataPointer(std::size_t i) override { return _data + i; }
        const void* dataPointer(std::size_t i) const override { return _data + i; }

        uint32_t dimensions() const override { return 3; }

        uint32_t width() const override { return _width; }
        uint32_t height() const override { return _height; }
        uint32_t depth() const override { return _depth; }

        value_type* data() { return _data; }
        const value_type* data() const { return _data; }

        std::size_t index(uint32_t i, uint32_t j, uint32_t k) const noexcept { return static_cast<std::size_t>(k) * _width * _height + static_cast<std::size_t>(j) * _width + i; }

        value_type& operator[](std::size_t i) { return _data[i]; }
        const value_type& operator[](std::size_t i) const { return _data[i]; }

        value_type& at(std::size_t i) { return _data[i]; }
        const value_type& at(std::size_t i) const { return _data[i]; }

        value_type& operator()(uint32_t i, uint32_t j, uint32_t k) { return _data[index(i, j, k)]; }
        const value_type& operator()(uint32_t i, uint32_t j, uint32_t k) const { return _data[index(i, j, k)]; }

        value_type& at(uint32_t i, uint32_t j, uint32_t k) { return _data[index(i, j, k)]; }
        const value_type& at(uint32_t i, uint32_t j, uint32_t k) const { return _data[index(i, j, k)]; }

        void set(std::size_t i, const value_type& v) { _data[i] = v; }
        void set(uint32_t i, uint32_t j, uint32_t k, const value_type& v) { _data[index(i, j, k)] = v; }

        iterator begin() { return _data; }
        const_iterator begin() const { return _data; }

        iterator end() { return _data + size(); }
        const_iterator end() const { return _data + size(); }

    protected:
        virtual ~Array3D()
        {
            if (_data) delete[] _data;
        }

    private:
        uint32_t _width;
        uint32_t _height;
        uint32_t _depth;
        value_type* _data;
    };

    VSG_array3D(ubyteArray3D, std::uint8_t);
    VSG_array3D(ushortArray3D, std::uint16_t);
    VSG_array3D(uintArray3D, uint32_t);
    VSG_array3D(floatArray3D, float);
    VSG_array3D(doubleArray3D, double);

    VSG_array3D(vec2Array3D, vec2);
    VSG_array3D(vec3Array3D, vec3);
    VSG_array3D(vec4Array3D, vec4);

    VSG_array3D(dvec2Array3D, dvec2);
    VSG_array3D(dvec3Array3D, dvec3);
    VSG_array3D(dvec4Array3D, dvec4);

    VSG_array3D(ubvec2Array3D, ubvec2);
    VSG_array3D(ubvec3Array3D, ubvec3);
    VSG_array3D(ubvec4Array3D, ubvec4);

    VSG_array3D(block64Array3D, block64);
    VSG_array3D(block128Array3D, block128);

} // namespace vsg
