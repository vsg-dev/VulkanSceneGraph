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
        using iterator = stride_iterator<value_type>;
        using const_iterator = stride_iterator<const value_type>;

        Array3D() :
            _data(nullptr),
            _width(0),
            _height(0),
            _depth(0) {}

        Array3D(const Array3D& rhs) :
            Data(rhs.properties, sizeof(value_type)),
            _data(nullptr),
            _width(rhs._width),
            _height(rhs._height),
            _depth(rhs._depth)
        {
            if (_width != 0 && _height != 0 && _depth != 0)
            {
                _data = _allocate(_width * _height * _depth);
                auto dest_v = _data;
                for (auto& v : rhs) *(dest_v++) = v;
            }
            dirty();
        }

        Array3D(uint32_t width, uint32_t height, uint32_t depth, Properties in_properties = {}) :
            Data(in_properties, sizeof(value_type)),
            _data(_allocate(width * height * depth)),
            _width(width),
            _height(height),
            _depth(depth)
        {
            dirty();
        }

        Array3D(uint32_t width, uint32_t height, uint32_t depth, value_type* data, Properties in_properties = {}) :
            Data(in_properties, sizeof(value_type)),
            _data(data),
            _width(width),
            _height(height),
            _depth(depth) { dirty(); }

        Array3D(uint32_t width, uint32_t height, uint32_t depth, const value_type& value, Properties in_properties = {}) :
            Data(in_properties, sizeof(value_type)),
            _data(_allocate(width * height * depth)),
            _width(width),
            _height(height),
            _depth(depth)
        {
            for (auto& v : *this) v = value;
            dirty();
        }

        Array3D(ref_ptr<Data> data, uint32_t offset, uint32_t stride, uint32_t width, uint32_t height, uint32_t depth, Properties in_properties = {}) :
            Data(),
            _data(nullptr),
            _width(0),
            _height(0),
            _depth(0)
        {
            assign(data, offset, stride, width, height, depth, in_properties);
        }

        template<typename... Args>
        static ref_ptr<Array3D> create(Args... args)
        {
            return ref_ptr<Array3D>(new Array3D(args...));
        }

        std::size_t sizeofObject() const noexcept override { return sizeof(Array3D); }
        const char* className() const noexcept override { return type_name<Array3D>(); }
        const std::type_info& type_info() const noexcept override { return typeid(*this); }
        bool is_compatible(const std::type_info& type) const noexcept override { return typeid(Array3D) == type || Data::is_compatible(type); }

        // implementation provided by Visitor.h
        void accept(Visitor& visitor) override;
        void accept(ConstVisitor& visitor) const override;

        void read(Input& input) override
        {
            std::size_t original_size = size();

            Data::read(input);

            uint32_t w = input.readValue<uint32_t>("width");
            uint32_t h = input.readValue<uint32_t>("height");
            uint32_t d = input.readValue<uint32_t>("depth");

            if (auto data_storage = input.readObject<Data>("storage"))
            {
                uint32_t offset = input.readValue<uint32_t>("offset");
                assign(data_storage, offset, properties.stride, w, h, d, properties);
                return;
            }

            if (input.matchPropertyName("data"))
            {
                std::size_t new_size = computeValueCountIncludingMipmaps(w, h, d, properties.maxNumMipmaps);

                if (_data) // if data already may be able to reuse it
                {
                    if (original_size != new_size) // if existing data is a different size delete old, and create new
                    {
                        _delete();
                        _data = _allocate(new_size);
                    }
                }
                else // allocate space for data
                {
                    _data = _allocate(new_size);
                }

                properties.stride = sizeof(value_type);
                _width = w;
                _height = h;
                _depth = d;
                _storage = nullptr;

                if (_data) input.read(new_size, _data);

                dirty();
            }
        }

        void write(Output& output) const override
        {
            Data::write(output);

            output.writeValue<uint32_t>("width", _width);
            output.writeValue<uint32_t>("height", _height);
            output.writeValue<uint32_t>("depth", _depth);

            output.writeObject("storage", _storage);
            if (_storage)
            {
                auto offset = (reinterpret_cast<uintptr_t>(_data) - reinterpret_cast<uintptr_t>(_storage->dataPointer()));
                output.writeValue<uint32_t>("offset", offset);
                return;
            }

            output.writePropertyName("data");
            output.write(valueCount(), _data);
            output.writeEndOfLine();
        }

        std::size_t size() const { return (properties.maxNumMipmaps <= 1) ? (static_cast<std::size_t>(_width) * _height * _depth) : computeValueCountIncludingMipmaps(_width, _height, _depth, properties.maxNumMipmaps); }

        bool available() const { return _data != nullptr; }
        bool empty() const { return _data == nullptr; }

        void clear()
        {
            _delete();

            _width = 0;
            _height = 0;
            _depth = 0;
            _data = nullptr;
            _storage = nullptr;
        }

        Array3D& operator=(const Array3D& rhs)
        {
            if (&rhs == this) return *this;

            clear();

            properties = rhs.properties;
            _width = rhs._width;
            _height = rhs._height;
            _depth = rhs._depth;

            if (_width != 0 && _height != 0 && _depth != 0)
            {
                _data = _allocate(_width * _height * _depth);
                auto dest_v = _data;
                for (auto& v : rhs) *(dest_v++) = v;
            }

            dirty();

            return *this;
        }

        void assign(uint32_t width, uint32_t height, uint32_t depth, value_type* data, Properties in_properties = {})
        {
            _delete();

            properties = in_properties;
            properties.stride = sizeof(value_type);
            _width = width;
            _height = height;
            _depth = depth;
            _data = data;
            _storage = nullptr;

            dirty();
        }

        void assign(ref_ptr<Data> storage, uint32_t offset, uint32_t stride, uint32_t width, uint32_t height, uint32_t depth, Properties in_properties = {})
        {
            _delete();

            _storage = storage;
            properties = in_properties;
            properties.stride = stride;
            if (_storage && _storage->dataPointer())
            {
                _data = reinterpret_cast<value_type*>(reinterpret_cast<uint8_t*>(_storage->dataPointer()) + offset);
                _width = width;
                _height = height;
                _depth = depth;
            }
            else
            {
                _data = nullptr;
                _width = 0;
                _height = 0;
                _depth = 0;
            }

            dirty();
        }

        // release the data so that ownership can be passed on, the local data pointer and size is set to 0 and destruction of Array will no result in the data being deleted.
        // when the data is stored in a separate vsg::Data object then return nullptr and do not attempt to release data.
        void* dataRelease() override
        {
            if (!_storage)
            {
                void* tmp = _data;
                _data = nullptr;
                _width = 0;
                _height = 0;
                _depth = 0;
                return tmp;
            }
            else
            {
                return nullptr;
            }
        }

        std::size_t valueSize() const override { return sizeof(value_type); }
        std::size_t valueCount() const override { return size(); }

        bool dataAvailable() const override { return available(); }
        std::size_t dataSize() const override { return size() * properties.stride; }

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

        inline value_type* data(std::size_t i) { return reinterpret_cast<value_type*>(reinterpret_cast<uint8_t*>(_data) + i * properties.stride); }
        inline const value_type* data(std::size_t i) const { return reinterpret_cast<const value_type*>(reinterpret_cast<const uint8_t*>(_data) + i * properties.stride); }

        std::size_t index(uint32_t i, uint32_t j, uint32_t k) const noexcept { return static_cast<std::size_t>(k * _width * _height + j * _width + i); }

        value_type& operator[](std::size_t i) { return *data(i); }
        const value_type& operator[](std::size_t i) const { return *data(i); }

        value_type& at(std::size_t i) { return *data(i); }
        const value_type& at(std::size_t i) const { return *data(i); }

        value_type& operator()(uint32_t i, uint32_t j, uint32_t k) { return *data(index(i, j, k)); }
        const value_type& operator()(uint32_t i, uint32_t j, uint32_t k) const { return *data(index(i, j, k)); }

        value_type& at(uint32_t i, uint32_t j, uint32_t k) { return *data(index(i, j, k)); }
        const value_type& at(uint32_t i, uint32_t j, uint32_t k) const { return *data(index(i, j, k)); }

        void set(std::size_t i, const value_type& v) { *data(i) = v; }
        void set(uint32_t i, uint32_t j, uint32_t k, const value_type& v) { *data(index(i, j, k)) = v; }

        Data* storage() { return _storage; }
        const Data* storage() const { return _storage; }

        iterator begin() { return iterator{_data, properties.stride}; }
        const_iterator begin() const { return const_iterator{_data, properties.stride}; }

        iterator end() { return iterator{data(_width * _height * _depth), properties.stride}; }
        const_iterator end() const { return const_iterator{data(_width * _height * _depth), properties.stride}; }

    protected:
        virtual ~Array3D()
        {
            _delete();
        }

        value_type* _allocate(size_t size) const
        {
            if (size == 0)
                return nullptr;
            else if (properties.allocatorType == ALLOCATOR_TYPE_NEW_DELETE)
                return new value_type[size];
            else if (properties.allocatorType == ALLOCATOR_TYPE_MALLOC_FREE)
                return new (std::malloc(sizeof(value_type) * size)) value_type[size];
            else
                return new (vsg::allocate(sizeof(value_type) * size, ALLOCATOR_AFFINITY_DATA)) value_type[size];
        }

        void _delete()
        {
            if (!_storage && _data)
            {
                if (properties.allocatorType == ALLOCATOR_TYPE_NEW_DELETE)
                    delete[] _data;
                else if (properties.allocatorType == ALLOCATOR_TYPE_MALLOC_FREE)
                    std::free(_data);
                else if (properties.allocatorType != 0)
                    vsg::deallocate(_data);
            }
        }

    private:
        value_type* _data;
        uint32_t _width;
        uint32_t _height;
        uint32_t _depth;
        ref_ptr<Data> _storage;
    };

    VSG_array3D(byteArray3D, int8_t);
    VSG_array3D(ubyteArray3D, uint8_t);
    VSG_array3D(shortArray3D, int16_t);
    VSG_array3D(ushortArray3D, uint16_t);
    VSG_array3D(intArray3D, int32_t);
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
