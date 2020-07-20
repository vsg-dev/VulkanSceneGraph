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

        explicit Array(uint32_t numElements, Layout layout = {}) :
            Data(layout, sizeof(value_type)),
            _data(new value_type[numElements]),
            _size(numElements) {}

        Array(uint32_t numElements, value_type* data, Layout layout = {}) :
            Data(layout, sizeof(value_type)),
            _data(data),
            _size(numElements) {}

        Array(uint32_t numElements, const value_type& value, Layout layout = {}) :
            Data(layout, sizeof(value_type)),
            _data(new value_type[numElements]),
            _size(numElements)
        {
            for (auto& v : *this) v = value;
        }

        Array(ref_ptr<Data> data, uint32_t offset, uint32_t stride, uint32_t numElements, Layout layout = Layout()):
            Data(),
            _data(nullptr),
            _size(0)
        {
            assign(data, offset, stride, numElements, layout);
        }

        explicit Array(std::initializer_list<value_type> l) :
            _data(new value_type[l.size()]),
            _size(static_cast<uint32_t>(l.size()))
        {
            _layout.stride = sizeof(value_type);
            value_type* ptr = _data;
            for (const value_type& v : l) { (*ptr++) = v; }
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

        std::size_t sizeofObject() const noexcept override { return sizeof(Array); }

        // implementation provided by Visitor.h
        void accept(Visitor& visitor) override;
        void accept(ConstVisitor& visitor) const override;

        const char* className() const noexcept override { return type_name<Array>(); }

        void read(Input& input) override
        {
            std::size_t original_total_size = size();

            Data::read(input);
            uint32_t width_size = input.readValue<uint32_t>("Size");
            std::size_t new_total_size = computeValueCountIncludingMipmaps(width_size, 1, 1, _layout.maxNumMipmaps);

            if (input.matchPropertyName("Data"))
            {
                if (_data) // if data already may be able to reuse it
                {
                    if (original_total_size != new_total_size) // if existing data is a different size delete old, and create new
                    {
                        delete[] _data;
                        _data = new value_type[new_total_size];
                    }
                }
                else // allocate space for data
                {
                    _data = new value_type[new_total_size];
                }

                _layout.stride = sizeof(value_type);
                _size = width_size;
                _storage = nullptr;

                input.read(new_total_size, _data);
            }
        }

        void write(Output& output) const override
        {
            Data::write(output);
            output.writeValue<uint32_t>("Size", _size);

            output.writePropertyName("Data");
            output.write(size(), _data);
            output.writeEndOfLine();
        }

        std::size_t size() const { return (_layout.maxNumMipmaps <= 1) ? _size : computeValueCountIncludingMipmaps(_size, 1, 1, _layout.maxNumMipmaps); }

        bool empty() const { return _size == 0; }

        void clear()
        {
            _delete();

            _size = 0;
            _data = nullptr;
            _storage = nullptr;
        }

        void assign(uint32_t numElements, value_type* data, Layout layout = Layout())
        {
            _delete();

            _layout = layout;
            _layout.stride = sizeof(value_type);
            _size = numElements;
            _data = data;
            _storage = nullptr;
        }

        void assign(ref_ptr<Data> storage, uint32_t offset, uint32_t stride, uint32_t numElements, Layout layout = Layout())
        {
            _delete();

            _storage = storage;
            _layout = layout;
            _layout.stride = stride;
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
        }

        // release the data so that ownership can be passed on, the local data pointer and size is set to 0 and destruction of Array will no result in the data being deleted.
        // when the data is stored in a sperate vsg::Data object then return nullptr and do not attempt to release data.
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

        std::size_t dataSize() const override { return size() * _layout.stride; }

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

        inline value_type* data(std::size_t i) { return reinterpret_cast<value_type*>(reinterpret_cast<uint8_t*>(_data) + i*_layout.stride); }
        inline const value_type* data(std::size_t i) const { return reinterpret_cast<const value_type*>(reinterpret_cast<const uint8_t*>(_data) + i*_layout.stride); }

        value_type& operator[](std::size_t i) { return *data(i); }
        const value_type& operator[](std::size_t i) const  { return *data(i); }

        value_type& at(std::size_t i) { return *data(i); }
        const value_type& at(std::size_t i) const { return *data(i); }

        void set(std::size_t i, const value_type& v) { *data(i) = v; }

        Data* storage() { return _storage; }
        const Data* storage() const { return _storage; }

        iterator begin() { return iterator{_data, _layout.stride}; }
        const_iterator begin() const { return const_iterator{_data, _layout.stride}; }

        iterator end() { return iterator{data(_size), _layout.stride}; }
        const_iterator end() const { return const_iterator{data(_size), _layout.stride}; }

    protected:
        virtual ~Array()
        {
            _delete();
        }

        void _delete()
        {
            if (!_storage && _data) delete[] _data;
        }

    private:
        value_type* _data;
        uint32_t _size;
        ref_ptr<Data> _storage;
    };

    VSG_array(ubyteArray, uint8_t);
    VSG_array(ushortArray, uint16_t);
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
