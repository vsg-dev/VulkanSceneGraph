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
    template<typename T, typename P>
    struct stride_iterator
    {
        using value_type = T;

        value_type* ptr;
        uint32_t stride; // stride in bytes

        inline void advance() { ptr = reinterpret_cast<value_type*>(reinterpret_cast<P*>(ptr) + stride); }

        stride_iterator& operator++() { advance(); return *this; }
        stride_iterator operator++(int) { stride_iterator reval(*this); advance(); return *this; }
        bool operator==(stride_iterator rhs) const { return ptr == rhs.ptr; }
        bool operator!=(stride_iterator rhs) const { return ptr != rhs.ptr; }

        value_type& operator*() { return *reinterpret_cast<value_type*>(ptr); }
        value_type* operator->() { return reinterpret_cast<value_type*>(ptr); }
    };

    template<typename T>
    class Array : public Data
    {
    public:
        using value_type = T;

        using iterator = stride_iterator<value_type, uint8_t>;
        using const_iterator = stride_iterator<const value_type, const uint8_t>;

        Array() :
            _data(nullptr),
            _stride(0),
            _size(0) {}

        explicit Array(std::uint32_t numElements, Layout layout = {}) :
            Data(layout),
            _data(new value_type[numElements]),
            _stride(sizeof(value_type)),
            _size(numElements) {}

        Array(std::uint32_t numElements, value_type* data, Layout layout = {}) :
            Data(layout),
            _data(data),
            _stride(sizeof(value_type)),
            _size(numElements) {}

        Array(std::uint32_t numElements, const value_type& value, Layout layout = {}) :
            Data(layout),
            _data(new value_type[numElements]),
            _stride(sizeof(value_type)),
            _size(numElements)
        {
            for (auto& v : *this) v = value;
        }

        Array(ref_ptr<Data> data, std::uint32_t numElements, uint32_t offset, uint32_t stride, Layout layout = Layout()):
            Data(),
            _data(nullptr),
            _stride(0),
            _size(0)
        {
            assign(data, numElements, offset, stride, layout);
        }

        explicit Array(std::initializer_list<value_type> l) :
            _data(new value_type[l.size()]),
            _stride(sizeof(value_type)),
            _size(static_cast<std::uint32_t>(l.size()))
        {
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
            std::uint32_t width_size = input.readValue<std::uint32_t>("Size");
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

                _size = width_size;

                input.read(new_total_size, _data);
            }
        }

        void write(Output& output) const override
        {
            Data::write(output);
            output.writeValue<std::uint32_t>("Size", _size);

            output.writePropertyName("Data");
            output.write(size(), _data);
            output.writeEndOfLine();
        }

        std::size_t size() const { return (_layout.maxNumMipmaps <= 1) ? _size : computeValueCountIncludingMipmaps(_size, 1, 1, _layout.maxNumMipmaps); }

        bool empty() const { return _size == 0; }

        // should Array be fixed size?
        void clear()
        {
            _size = 0;
            if (_data) { delete[] _data; }
            _data = nullptr;
        }

        void assign(std::uint32_t numElements, value_type* data, Layout layout = Layout())
        {
            if (!_storage && _data != nullptr) delete[] _data;

            _layout = layout;
            _data = data;
            _stride = sizeof(value_type);
            _size = numElements;
            _storage = nullptr;
        }

        void assign(ref_ptr<Data> storage, std::uint32_t numElements, uint32_t offset, uint32_t stride, Layout layout = Layout())
        {
            if (!_storage && _data != nullptr) delete[] _data;

            _storage = storage;
            _stride = stride;
            _layout = layout;
            if (_storage && _storage->dataPointer())
            {
                _data = static_cast<value_type*>(_storage->dataPointer()) + offset;
                _size = numElements;
            }
            else
            {
                _data = nullptr;
                _size = 0;
            }
        }

        // release the data so that ownership can be passed on, the local data pointer and size is set to 0 and destruction of Array will no result in the data being deleted.
        void* dataRelease() override
        {
            void* tmp = _data;
            _data = nullptr;
            _size = 0;
            return tmp;
        }

        std::size_t valueSize() const override { return sizeof(value_type); }
        std::size_t valueCount() const override { return size(); }

        std::size_t dataSize() const override { return size() * sizeof(value_type); }

        void* dataPointer() override { return data(); }
        const void* dataPointer() const override { return data(); }

        void* dataPointer(std::size_t i) override { return data(i); }
        const void* dataPointer(std::size_t i) const override { return data(i); }

        std::uint32_t dimensions() const override { return 1; }

        std::uint32_t width() const override { return _size; }
        std::uint32_t height() const override { return 1; }
        std::uint32_t depth() const override { return 1; }

        bool continigous() const { return sizeof(value_type) == _stride; }

        value_type* data() { return _data; }
        const value_type* data() const { return _data; }

        inline value_type* data(std::size_t i) { return reinterpret_cast<value_type*>(reinterpret_cast<uint8_t*>(_data) + i*_stride); }
        inline const value_type* data(std::size_t i) const { return reinterpret_cast<const value_type*>(reinterpret_cast<const uint8_t*>(_data) + i*_stride); }

        Data* storage() { return _storage; }
        const Data* storage() const { return _storage; }

        value_type& operator[](std::size_t i) { return *data(i); }
        const value_type& operator[](std::size_t i) const  { return *data(i); }

        value_type& at(std::size_t i) { return *data(i); }
        const value_type& at(std::size_t i) const { return *data(i); }

        void set(std::size_t i, const value_type& v) { *data(i) = v; }

        iterator begin() { return iterator{_data, _stride}; }
        const_iterator begin() const { return const_iterator{_data, _stride}; }

        iterator end() { return iterator{data(_size), _stride}; }
        const_iterator end() const { return const_iterator{data(_size), _stride}; }

    protected:
        virtual ~Array()
        {
            if (!_storage && _data) delete[] _data;
        }

    private:
        value_type* _data;
        std::uint32_t _stride;
        std::uint32_t _size;
        ref_ptr<Data> _storage;
    };

    VSG_array(ubyteArray, std::uint8_t);
    VSG_array(ushortArray, std::uint16_t);
    VSG_array(uintArray, std::uint32_t);
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
