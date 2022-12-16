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

#define VSG_array(N, T) \
    using N = Array<T>; \
    template<>          \
    constexpr const char* type_name<N>() noexcept { return "vsg::" #N; }

namespace vsg
{

    template<typename T>
    class Array : public Data
    {
    public:
        using value_type = T;
        using iterator = stride_iterator<value_type>;
        using const_iterator = stride_iterator<const value_type>;

        Array() :
            _data(nullptr),
            _size(0) {}

        Array(const Array& rhs) :
            Data(rhs.properties, sizeof(value_type)),
            _data(nullptr),
            _size(rhs._size)
        {
            if (_size != 0)
            {
                _data = _allocate(_size);
                auto dest_v = _data;
                for (auto& v : rhs) *(dest_v++) = v;
            }
            dirty();
        }

        explicit Array(uint32_t numElements, Properties in_properties = {}) :
            Data(in_properties, sizeof(value_type)),
            _data(_allocate(numElements)),
            _size(numElements) { dirty(); }

        Array(uint32_t numElements, value_type* data, Properties in_properties = {}) :
            Data(in_properties, sizeof(value_type)),
            _data(data),
            _size(numElements) { dirty(); }

        Array(uint32_t numElements, const value_type& value, Properties in_properties = {}) :
            Data(in_properties, sizeof(value_type)),
            _data(_allocate(numElements)),
            _size(numElements)
        {
            for (auto& v : *this) v = value;
            dirty();
        }

        Array(ref_ptr<Data> data, uint32_t offset, uint32_t stride, uint32_t numElements, Properties in_properties = {}) :
            Data(),
            _data(nullptr),
            _size(0)
        {
            assign(data, offset, stride, numElements, in_properties);
        }

        explicit Array(std::initializer_list<value_type> l) :
            _data(_allocate(l.size())),
            _size(static_cast<uint32_t>(l.size()))
        {
            properties.stride = sizeof(value_type);

            iterator itr = begin();
            for (const value_type& v : l) { (*itr++) = v; }

            dirty();
        }

        explicit Array(ref_ptr<Data> data, uint32_t offset, uint32_t stride, std::initializer_list<value_type> l) :
            _data(nullptr),
            _size(0)
        {
            assign(data, offset, stride, l.size());

            iterator itr = begin();
            for (const value_type& v : l) { (*itr++) = v; }

            dirty();
        }

        template<typename... Args>
        static ref_ptr<Array> create(Args... args)
        {
            return ref_ptr<Array>(new Array(args...));
        }

        static ref_ptr<Array> create(std::initializer_list<value_type> l)
        {
            return ref_ptr<Array>(new Array(l));
        }

        static ref_ptr<Array> create(ref_ptr<Data> data, uint32_t offset, uint32_t stride, std::initializer_list<value_type> l)
        {
            return ref_ptr<Array>(new Array(data, offset, stride, l));
        }

        std::size_t sizeofObject() const noexcept override { return sizeof(Array); }
        const char* className() const noexcept override { return type_name<Array>(); }
        const std::type_info& type_info() const noexcept override { return typeid(*this); }
        bool is_compatible(const std::type_info& type) const noexcept override { return typeid(Array) == type || Data::is_compatible(type); }

        // implementation provided by Visitor.h
        void accept(Visitor& visitor) override;
        void accept(ConstVisitor& visitor) const override;

        void read(Input& input) override
        {
            std::size_t original_total_size = size();

            Data::read(input);

            uint32_t width_size = input.readValue<uint32_t>("size");

            if (auto data_storage = input.readObject<Data>("storage"))
            {
                uint32_t offset = input.readValue<uint32_t>("offset");
                assign(data_storage, offset, properties.stride, width_size, properties);
                return;
            }

            if (input.matchPropertyName("data"))
            {
                std::size_t new_total_size = computeValueCountIncludingMipmaps(width_size, 1, 1, properties.maxNumMipmaps);

                if (_data) // if data already may be able to reuse it
                {
                    if (original_total_size != new_total_size) // if existing data is a different size delete old, and create new
                    {
                        clear();

                        _data = _allocate(new_total_size);
                    }
                }
                else // allocate space for data
                {
                    _data = _allocate(new_total_size);
                }

                properties.stride = sizeof(value_type);
                _size = width_size;
                _storage = nullptr;

                if (_data) input.read(new_total_size, _data);

                dirty();
            }
        }

        void write(Output& output) const override
        {
            Data::write(output);

            output.writeValue<uint32_t>("size", _size);
            output.writeObject("storage", _storage);
            if (_storage)
            {
                auto offset = (reinterpret_cast<uintptr_t>(_data) - reinterpret_cast<uintptr_t>(_storage->dataPointer()));
                output.writeValue<uint32_t>("offset", offset);
                return;
            }

            output.writePropertyName("data");
            output.write(size(), _data);
            output.writeEndOfLine();
        }

        std::size_t size() const { return (properties.maxNumMipmaps <= 1) ? _size : computeValueCountIncludingMipmaps(_size, 1, 1, properties.maxNumMipmaps); }

        bool available() const { return _data != nullptr; }
        bool empty() const { return _data == nullptr; }

        void clear()
        {
            _delete();

            _size = 0;
            _data = nullptr;
            _storage = nullptr;
        }

        Array& operator=(const Array& rhs)
        {
            if (&rhs == this) return *this;

            clear();

            properties = rhs.properties;
            _size = rhs._size;

            if (_size != 0)
            {
                _data = _allocate(_size);
                auto dest_v = _data;
                for (auto& v : rhs) *(dest_v++) = v;
            }

            dirty();

            return *this;
        }

        void assign(uint32_t numElements, value_type* data, Properties in_properties = {})
        {
            _delete();

            properties = in_properties;
            properties.stride = sizeof(value_type);
            _size = numElements;
            _data = data;
            _storage = nullptr;

            dirty();
        }

        void assign(ref_ptr<Data> storage, uint32_t offset, uint32_t stride, uint32_t numElements, Properties in_properties = {})
        {
            _delete();

            _storage = storage;
            properties = in_properties;
            properties.stride = stride;
            if (_storage && _storage->dataPointer())
            {
                _data = reinterpret_cast<value_type*>(reinterpret_cast<uint8_t*>(_storage->dataPointer()) + offset);
                _size = numElements;
            }
            else
            {
                _data = nullptr;
                _size = 0;
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
                _size = 0;
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

        uint32_t dimensions() const override { return 1; }

        uint32_t width() const override { return _size; }
        uint32_t height() const override { return 1; }
        uint32_t depth() const override { return 1; }

        value_type* data() { return _data; }
        const value_type* data() const { return _data; }

        inline value_type* data(std::size_t i) { return reinterpret_cast<value_type*>(reinterpret_cast<uint8_t*>(_data) + i * properties.stride); }
        inline const value_type* data(std::size_t i) const { return reinterpret_cast<const value_type*>(reinterpret_cast<const uint8_t*>(_data) + i * properties.stride); }

        value_type& operator[](std::size_t i) { return *data(i); }
        const value_type& operator[](std::size_t i) const { return *data(i); }

        value_type& at(std::size_t i) { return *data(i); }
        const value_type& at(std::size_t i) const { return *data(i); }

        void set(std::size_t i, const value_type& v) { *data(i) = v; }

        Data* storage() { return _storage; }
        const Data* storage() const { return _storage; }

        iterator begin() { return iterator{_data, properties.stride}; }
        const_iterator begin() const { return const_iterator{_data, properties.stride}; }

        iterator end() { return iterator{data(_size), properties.stride}; }
        const_iterator end() const { return const_iterator{data(_size), properties.stride}; }

    protected:
        virtual ~Array()
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
        uint32_t _size;
        ref_ptr<Data> _storage;
    };

    VSG_array(byteArray, int8_t);
    VSG_array(ubyteArray, uint8_t);
    VSG_array(shortArray, int16_t);
    VSG_array(ushortArray, uint16_t);
    VSG_array(intArray, int32_t);
    VSG_array(uintArray, uint32_t);
    VSG_array(floatArray, float);
    VSG_array(doubleArray, double);

    VSG_array(vec2Array, vec2);
    VSG_array(vec3Array, vec3);
    VSG_array(vec4Array, vec4);

    VSG_array(dvec2Array, dvec2);
    VSG_array(dvec3Array, dvec3);
    VSG_array(dvec4Array, dvec4);

    VSG_array(bvec2Array, bvec2);
    VSG_array(bvec3Array, bvec3);
    VSG_array(bvec4Array, bvec4);

    VSG_array(ubvec2Array, ubvec2);
    VSG_array(ubvec3Array, ubvec3);
    VSG_array(ubvec4Array, ubvec4);

    VSG_array(svec2Array, svec2);
    VSG_array(svec3Array, svec3);
    VSG_array(svec4Array, svec4);

    VSG_array(usvec2Array, usvec2);
    VSG_array(usvec3Array, usvec3);
    VSG_array(usvec4Array, usvec4);

    VSG_array(ivec2Array, ivec2);
    VSG_array(ivec3Array, ivec3);
    VSG_array(ivec4Array, ivec4);

    VSG_array(uivec2Array, uivec2);
    VSG_array(uivec3Array, uivec3);
    VSG_array(uivec4Array, uivec4);

    VSG_array(mat4Array, mat4);
    VSG_array(dmat4Array, dmat4);

    VSG_array(block64Array, block64);
    VSG_array(block128Array, block128);

} // namespace vsg
