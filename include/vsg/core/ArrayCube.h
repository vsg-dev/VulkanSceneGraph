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

#define VSG_arrayCube(N, T) \
    using N = ArrayCube<T>; \
    template<>            \
    constexpr const char* type_name<N>() noexcept { return "vsg::" #N; }

namespace vsg
{
    template<typename T>
    class ArrayCube : public Data
    {
    public:
        using value_type = T;
        using iterator = stride_iterator<value_type>;
        using const_iterator = stride_iterator<const value_type>;

        ArrayCube() :
            _data(nullptr),
            _width(0),
            _height(0),
            _depth(6) {}

        ArrayCube(uint32_t width, uint32_t height, Layout layout = {}) :
            Data(layout, sizeof(value_type)),
            _data(new value_type[width * height * 6]),
            _width(width),
            _height(height),
            _depth(6) {}

        ArrayCube(uint32_t width, uint32_t height, value_type* data, Layout layout = {}) :
            Data(layout, sizeof(value_type)),
            _data(data),
            _width(width),
            _height(height),
            _depth(6) {}

        ArrayCube(uint32_t width, uint32_t height, const value_type& value, Layout layout = {}) :
            Data(layout, sizeof(value_type)),
            _data(new value_type[width * height * 6]),
            _width(width),
            _height(height),
            _depth(6)
        {
            for (auto& v : *this) v = value;
        }

        ArrayCube(ref_ptr<Data> data, uint32_t offset, uint32_t stride, uint32_t width, uint32_t height, Layout layout = Layout()) :
            Data(),
            _data(nullptr),
            _width(0),
            _height(0),
            _depth(6)
        {
            assign(data, offset, stride, width, height, layout);
        }

        template<typename... Args>
        static ref_ptr<ArrayCube> create(Args... args)
        {
            return ref_ptr<ArrayCube>(new ArrayCube(args...));
        }

        std::size_t sizeofObject() const noexcept override { return sizeof(ArrayCube); }
        const char* className() const noexcept override { return type_name<ArrayCube>(); }
        const std::type_info& type_info() const noexcept override { return typeid(*this); }
        bool is_compatible(const std::type_info& type) const noexcept override { return typeid(ArrayCube) == type ? true : Data::is_compatible(type); }

        // implementation provided by Visitor.h
        void accept(Visitor& visitor) override;
        void accept(ConstVisitor& visitor) const override;

        void read(Input& input) override
        {
            std::size_t original_size = size();

            Data::read(input);

            uint32_t width = input.readValue<uint32_t>("Width");
            uint32_t height = input.readValue<uint32_t>("Height");

            if (input.version_greater_equal(0, 0, 1))
            {
                auto storage = input.readObject<Data>("Storage");
                if (storage)
                {
                    uint32_t offset = input.readValue<uint32_t>("Offset");
                    assign(storage, offset, _layout.stride, width, height, _layout);
                    return;
                }
            }

            if (input.matchPropertyName("Data"))
            {
                std::size_t new_size = computeValueCountIncludingMipmaps(width, height, 1, _layout.maxNumMipmaps) * 6;

                if (_data) // if data already may be able to reuse it
                {
                    if (original_size != new_size) // if existing data is a different size delete old, and create new
                    {
                        _delete();
                        _data = new value_type[new_size];
                    }
                }
                else // allocate space for data
                {
                    _data = new value_type[new_size];
                }

                _layout.stride = sizeof(value_type);
                _width = width;
                _height = height;
                _depth = 6;
                _storage = nullptr;

                input.read(new_size, _data);
            }
        }

        void write(Output& output) const override
        {
            Data::write(output);
            output.writeValue<uint32_t>("Width", _width);
            output.writeValue<uint32_t>("Height", _height);

            if (output.version_greater_equal(0, 0, 1))
            {
                output.writeObject("Storage", _storage);
                if (_storage)
                {
                    auto offset = (reinterpret_cast<uintptr_t>(_data) - reinterpret_cast<uintptr_t>(_storage->dataPointer()));
                    output.writeValue<uint32_t>("Offset", offset);
                    return;
                }
            }

            output.writePropertyName("Data");
            output.write(valueCount(), _data);
            output.writeEndOfLine();
        }

        std::size_t size() const { return (_layout.maxNumMipmaps <= 1) ? (static_cast<std::size_t>(_width) * _height * _depth) : computeValueCountIncludingMipmaps(_width, _height, 1, _layout.maxNumMipmaps) * 6; }

        bool empty() const { return _width == 0 && _height == 0 && _depth == 6; }

        void clear()
        {
            _delete();

            _width = 0;
            _height = 0;
            _depth = 6;
            _data = nullptr;
            _storage = nullptr;
        }

        void assign(uint32_t width, uint32_t height, value_type* data, Layout layout = Layout())
        {
            _delete();

            _layout = layout;
            _layout.stride = sizeof(value_type);
            _width = width;
            _height = height;
            _depth = 6;
            _data = data;
            _storage = nullptr;
        }

        void assign(ref_ptr<Data> storage, uint32_t offset, uint32_t stride, uint32_t width, uint32_t height, Layout layout = Layout())
        {
            _delete();

            _storage = storage;
            _layout = layout;
            _layout.stride = stride;
            if (_storage && _storage->dataPointer())
            {
                _data = reinterpret_cast<value_type*>(reinterpret_cast<uint8_t*>(_storage->dataPointer()) + offset);
                _width = width;
                _height = height;
                _depth = 6;
            }
            else
            {
                _data = nullptr;
                _width = 0;
                _height = 0;
                _depth = 6;
            }
        }

        // release the data so that ownership can be passed on, the local data pointer and size is set to 0 and destruction of Array will no result in the data being deleted.
        void* dataRelease() override
        {
            if (!_storage)
            {
                void* tmp = _data;
                _data = nullptr;
                _width = 0;
                _height = 0;
                _depth = 6;
                return tmp;
            }
            else
            {
                return nullptr;
            }
        }

        std::size_t valueSize() const override { return sizeof(value_type); }
        std::size_t valueCount() const override { return size(); }

        std::size_t dataSize() const override { return size() * _layout.stride; }

        void* dataPointer() override { return _data; }
        const void* dataPointer() const override { return _data; }

        void* dataPointer(std::size_t i) override { return data(i); }
        const void* dataPointer(std::size_t i) const override { return data(i); }

        uint32_t dimensions() const override { return 3; }

        uint32_t width() const override { return _width; }
        uint32_t height() const override { return _height; }
        uint32_t depth() const override { return _depth; }

        value_type* data() { return _data; }
        const value_type* data() const { return _data; }

        inline value_type* data(std::size_t i) { return reinterpret_cast<value_type*>(reinterpret_cast<uint8_t*>(_data) + i * _layout.stride); }
        inline const value_type* data(std::size_t i) const { return reinterpret_cast<const value_type*>(reinterpret_cast<const uint8_t*>(_data) + i * _layout.stride); }

        std::size_t index(uint32_t i, uint32_t j, uint32_t k) const noexcept { return static_cast<std::size_t>(k * _width * _height + j * _width + i); }

        value_type& operator[](std::size_t i) { return *data(i); }
        const value_type& operator[](std::size_t i) const { return *data(i); }

        value_type& at(std::size_t i) { return *data(i); }
        const value_type& at(std::size_t i) const { return *data(i); }

        value_type& operator()(uint32_t i, uint32_t j, uint32_t k) { return *data(index(i, j, k)); }
        const value_type& operator()(uint32_t i, uint32_t j, uint32_t k) const { return *data(index(i, j, k)); }

        value_type& at(uint32_t i, uint32_t j, uint32_t k) { return *data(index(i, j, k)); }
        const value_type& at(uint32_t i, uint32_t j, uint32_t k) const { return *data(index(i, j, k)); }

        void set(std::size_t i, const value_type& v) { data(i) = v; }
        void set(uint32_t i, uint32_t j, uint32_t k, const value_type& v) { *data(index(i, j, k)) = v; }

        Data* storage() { return _storage; }
        const Data* storage() const { return _storage; }

        iterator begin() { return iterator{_data, _layout.stride}; }
        const_iterator begin() const { return const_iterator{_data, _layout.stride}; }

        iterator end() { return iterator{data(_width * _height * _depth), _layout.stride}; }
        const_iterator end() const { return const_iterator{data(_width * _height * _depth), _layout.stride}; }

    protected:
        virtual ~ArrayCube()
        {
            _delete();
        }

        void _delete()
        {
            if (!_storage && _data) delete[] _data;
        }

    private:
        value_type* _data;
        uint32_t _width;
        uint32_t _height;
        uint32_t _depth;
        ref_ptr<Data> _storage;
    };

    VSG_arrayCube(byteArrayCube, int8_t);
    VSG_arrayCube(ubyteArrayCube, uint8_t);
    VSG_arrayCube(shortArrayCube, int16_t);
    VSG_arrayCube(ushortArrayCube, uint16_t);
    VSG_arrayCube(intArrayCube, int32_t);
    VSG_arrayCube(uintArrayCube, uint32_t);
    VSG_arrayCube(floatArrayCube, float);
    VSG_arrayCube(doubleArrayCube, double);
             
    VSG_arrayCube(vec2ArrayCube, vec2);
    VSG_arrayCube(vec3ArrayCube, vec3);
    VSG_arrayCube(vec4ArrayCube, vec4);
             
    VSG_arrayCube(dvec2ArrayCube, dvec2);
    VSG_arrayCube(dvec3ArrayCube, dvec3);
    VSG_arrayCube(dvec4ArrayCube, dvec4);
             
    VSG_arrayCube(ubvec2ArrayCube, ubvec2);
    VSG_arrayCube(ubvec3ArrayCube, ubvec3);
    VSG_arrayCube(ubvec4ArrayCube, ubvec4);
             
    VSG_arrayCube(block64ArrayCube, block64);
    VSG_arrayCube(block128ArrayCube, block128);

} // namespace vsg
