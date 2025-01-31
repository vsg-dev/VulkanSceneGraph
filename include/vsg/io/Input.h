#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Data.h>
#include <vsg/core/Object.h>
#include <vsg/core/Version.h>

#include <vsg/maths/box.h>
#include <vsg/maths/mat3.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/plane.h>
#include <vsg/maths/quat.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>

#include <vsg/io/FileSystem.h>
#include <vsg/io/ObjectFactory.h>

#include <set>
#include <unordered_map>

namespace vsg
{

    // forward declare
    class Options;

    /// Base class that provides a means of reading a range of data types from an input stream.
    /// Used by vsg::Object::read(Input&) implementations across the VSG to provide native serialization from binary/ascii files
    class VSG_DECLSPEC Input
    {
    public:
        Input(ref_ptr<ObjectFactory> in_objectFactory, ref_ptr<const Options> in_options = {});

        Input(const Input& output) = delete;
        Input& operator=(const Input& rhs) = delete;

        /// return true if property name matches the next token in the stream, or if property names are not required for format
        virtual bool matchPropertyName(const char* propertyName) = 0;

        // read value(s)
        virtual void read(size_t num, int8_t* value) = 0;
        virtual void read(size_t num, uint8_t* value) = 0;
        virtual void read(size_t num, int16_t* value) = 0;
        virtual void read(size_t num, uint16_t* value) = 0;
        virtual void read(size_t num, int32_t* value) = 0;
        virtual void read(size_t num, uint32_t* value) = 0;
        virtual void read(size_t num, int64_t* value) = 0;
        virtual void read(size_t num, uint64_t* value) = 0;
        virtual void read(size_t num, float* value) = 0;
        virtual void read(size_t num, double* value) = 0;
        virtual void read(size_t num, long double* value) = 0;
        virtual void read(size_t num, std::string* value) = 0;
        virtual void read(size_t num, std::wstring* value) = 0;
        virtual void read(size_t num, Path* value) = 0;

        // read object
        virtual ref_ptr<Object> read() = 0;

        // map char to int8_t
        void read(size_t num, char* value) { read(num, reinterpret_cast<int8_t*>(value)); }
        void read(size_t num, bool* value) { read(num, reinterpret_cast<int8_t*>(value)); }

        // vec/mat versions of read methods
        void read(size_t num, vec2* value) { read(num * value->size(), value->data()); }
        void read(size_t num, vec3* value) { read(num * value->size(), value->data()); }
        void read(size_t num, vec4* value) { read(num * value->size(), value->data()); }
        void read(size_t num, dvec2* value) { read(num * value->size(), value->data()); }
        void read(size_t num, dvec3* value) { read(num * value->size(), value->data()); }
        void read(size_t num, ldvec4* value) { read(num * value->size(), value->data()); }
        void read(size_t num, ldvec2* value) { read(num * value->size(), value->data()); }
        void read(size_t num, ldvec3* value) { read(num * value->size(), value->data()); }
        void read(size_t num, dvec4* value) { read(num * value->size(), value->data()); }
        void read(size_t num, bvec2* value) { read(num * value->size(), value->data()); }
        void read(size_t num, bvec3* value) { read(num * value->size(), value->data()); }
        void read(size_t num, bvec4* value) { read(num * value->size(), value->data()); }
        void read(size_t num, ubvec2* value) { read(num * value->size(), value->data()); }
        void read(size_t num, ubvec3* value) { read(num * value->size(), value->data()); }
        void read(size_t num, ubvec4* value) { read(num * value->size(), value->data()); }
        void read(size_t num, svec2* value) { read(num * value->size(), value->data()); }
        void read(size_t num, svec3* value) { read(num * value->size(), value->data()); }
        void read(size_t num, svec4* value) { read(num * value->size(), value->data()); }
        void read(size_t num, usvec2* value) { read(num * value->size(), value->data()); }
        void read(size_t num, usvec3* value) { read(num * value->size(), value->data()); }
        void read(size_t num, usvec4* value) { read(num * value->size(), value->data()); }
        void read(size_t num, ivec2* value) { read(num * value->size(), value->data()); }
        void read(size_t num, ivec3* value) { read(num * value->size(), value->data()); }
        void read(size_t num, ivec4* value) { read(num * value->size(), value->data()); }
        void read(size_t num, uivec2* value) { read(num * value->size(), value->data()); }
        void read(size_t num, uivec3* value) { read(num * value->size(), value->data()); }
        void read(size_t num, uivec4* value) { read(num * value->size(), value->data()); }
        void read(size_t num, quat* value) { read(num * value->size(), value->data()); }
        void read(size_t num, dquat* value) { read(num * value->size(), value->data()); }
        void read(size_t num, mat3* value) { read(num * value->size(), value->data()); }
        void read(size_t num, dmat3* value) { read(num * value->size(), value->data()); }
        void read(size_t num, mat4* value) { read(num * value->size(), value->data()); }
        void read(size_t num, dmat4* value) { read(num * value->size(), value->data()); }
        void read(size_t num, sphere* value) { read(num * value->size(), value->data()); }
        void read(size_t num, dsphere* value) { read(num * value->size(), value->data()); }
        void read(size_t num, box* value) { read(num * value->size(), value->data()); }
        void read(size_t num, dbox* value) { read(num * value->size(), value->data()); }
        void read(size_t num, plane* value) { read(num * value->size(), value->data()); }
        void read(size_t num, dplane* value) { read(num * value->size(), value->data()); }

        /// treat non standard type as raw data
        template<typename T>
        void read(size_t num, T* value)
        {
            if constexpr (has_read_write<T>())
            {
                for (size_t i = 0; i < num; ++i) value[i].read(*this);
            }
            else
            {
                read(num * sizeof(T), reinterpret_cast<uint8_t*>(value));
            }
        }

        template<typename T>
        void read(const char* propertyName, ref_ptr<T>& arg)
        {
            if (!matchPropertyName(propertyName)) return;
            arg = read().cast<T>();
        }

        template<typename T>
        void readObjects(const char* propertyName, T& values)
        {
            if (!matchPropertyName(propertyName)) return;

            uint32_t numElements = 0;
            read(1, &numElements);
            values.resize(numElements);

            using element_type = typename T::value_type::element_type;
            const char* element_name = type_name<element_type>();

            for (uint32_t i = 0; i < numElements; ++i)
            {
                read(element_name, values[i]);
            }
        }

        template<typename T>
        void readValues(const char* propertyName, std::vector<T>& values)
        {
            if (!matchPropertyName(propertyName)) return;

            uint32_t numElements = 0;
            read(1, &numElements);
            values.resize(numElements);

            for (uint32_t i = 0; i < numElements; ++i)
            {
                read("element", values[i]);
            }
        }

        template<typename T>
        void readValues(const char* propertyName, std::set<T>& values)
        {
            if (!matchPropertyName(propertyName)) return;

            uint32_t numElements = 0;
            read(1, &numElements);

            for (uint32_t i = 0; i < numElements; ++i)
            {
                T v;
                read("element", v);
                values.insert(v);
            }
        }

        /// match property name and read value(s)
        template<typename... Args>
        void read(const char* propertyName, Args&... args)
        {
            if (!matchPropertyName(propertyName)) return;

            // use fold expression to expand arguments and map to appropriate read method
            (read(1, &(args)), ...);
        }

        /// read object of a particular type
        ref_ptr<Object> readObject(const char* propertyName)
        {
            if (!matchPropertyName(propertyName)) return ref_ptr<Object>();

            return read();
        }

        /// read object of a particular type
        template<class T>
        ref_ptr<T> readObject(const char* propertyName)
        {
            if (!matchPropertyName(propertyName)) return ref_ptr<T>();

            ref_ptr<Object> object = read();
            return ref_ptr<T>(dynamic_cast<T*>(object.get()));
        }

        /// read object of a particular type
        template<class T>
        void readObject(const char* propertyName, ref_ptr<T>& arg)
        {
            if (!matchPropertyName(propertyName)) return;

            arg = read().cast<T>();
        }

        /// read a value of particular type
        template<typename T>
        T readValue(const char* propertyName)
        {
            T v{};
            read(propertyName, v);
            return v;
        }

        /// read a value as a type, then cast it to another type
        template<typename W, typename T>
        void readValue(const char* propertyName, T& value)
        {
            W read_value{};
            read(propertyName, read_value);
            value = static_cast<T>(read_value);
        }

        using ObjectID = uint32_t;
        using ObjectIDMap = std::map<ObjectID, ref_ptr<Object>>;

        ObjectIDMap objectIDMap;
        ref_ptr<ObjectFactory> objectFactory;
        ref_ptr<const Options> options;
        Path filename;

        VsgVersion version;

        virtual bool version_less(uint32_t major, uint32_t minor, uint32_t patch, uint32_t soversion = 0) const;
        virtual bool version_greater_equal(uint32_t major, uint32_t minor, uint32_t patch, uint32_t soversion = 0) const;

    protected:
        virtual ~Input();
    };

    template<>
    inline void Input::readObject(const char* propertyName, ref_ptr<Object>& arg)
    {
        if (!matchPropertyName(propertyName)) return;

        arg = read();
    }

} // namespace vsg
