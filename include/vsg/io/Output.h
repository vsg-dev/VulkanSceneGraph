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
#include <vsg/core/type_name.h>

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

#include <set>
#include <unordered_map>

namespace vsg
{

    /// Base class that provides a means of writing out a range of data types to an output stream.
    /// Used by vsg::Object::write(Output&) implementations across the VSG to provide native serialization to binary/ascii files
    class VSG_DECLSPEC Output
    {
    public:
        Output();
        Output(ref_ptr<const Options> in_options);

        Output(const Output& output) = delete;
        Output& operator=(const Output& rhs) = delete;

        /// write property name if appropriate for format
        virtual void writePropertyName(const char* propertyName) = 0;

        /// write end of line character if required.
        virtual void writeEndOfLine() = 0;

        /// write contiguous array of value(s)
        virtual void write(size_t num, const int8_t* value) = 0;
        virtual void write(size_t num, const uint8_t* value) = 0;
        virtual void write(size_t num, const int16_t* value) = 0;
        virtual void write(size_t num, const uint16_t* value) = 0;
        virtual void write(size_t num, const int32_t* value) = 0;
        virtual void write(size_t num, const uint32_t* value) = 0;
        virtual void write(size_t num, const int64_t* value) = 0;
        virtual void write(size_t num, const uint64_t* value) = 0;
        virtual void write(size_t num, const float* value) = 0;
        virtual void write(size_t num, const double* value) = 0;
        virtual void write(size_t num, const long double* value) = 0;
        virtual void write(size_t num, const std::string* value) = 0;
        virtual void write(size_t num, const std::wstring* value) = 0;
        virtual void write(size_t num, const Path* value) = 0;

        /// write object
        virtual void write(const Object* object) = 0;

        /// map char to int8_t
        void write(size_t num, const char* value) { write(num, reinterpret_cast<const int8_t*>(value)); }
        void write(size_t num, const bool* value) { write(num, reinterpret_cast<const int8_t*>(value)); }

        // vec/mat versions of write methods
        void write(size_t num, const vec2* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const vec3* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const vec4* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const dvec2* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const dvec3* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const dvec4* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const ldvec2* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const ldvec3* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const ldvec4* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const bvec2* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const bvec3* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const bvec4* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const ubvec2* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const ubvec3* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const ubvec4* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const svec2* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const svec3* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const svec4* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const usvec2* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const usvec3* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const usvec4* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const ivec2* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const ivec3* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const ivec4* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const uivec2* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const uivec3* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const uivec4* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const quat* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const dquat* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const mat3* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const dmat3* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const mat4* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const dmat4* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const sphere* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const dsphere* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const box* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const dbox* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const plane* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const dplane* value) { write(num * value->size(), value->data()); }

        template<typename T>
        void write(size_t num, const T* value)
        {
            if constexpr (has_read_write<T>())
            {
                for (size_t i = 0; i < num; ++i) value[i].write(*this);
            }
            else
            {
                write(num * sizeof(T), reinterpret_cast<const uint8_t*>(value));
            }
        }

        template<typename T>
        void write(const char* propertyName, const ref_ptr<T>& object)
        {
            writePropertyName(propertyName);
            write(object);
        }

        template<typename T>
        void writeObjects(const char* propertyName, const T& values)
        {
            uint32_t numElements = static_cast<uint32_t>(values.size());
            write(propertyName, numElements);

            using element_type = typename T::value_type::element_type;
            const char* element_name = type_name<element_type>();

            for (uint32_t i = 0; i < numElements; ++i)
            {
                write(element_name, values[i]);
            }
        }

        template<typename T>
        void writeValues(const char* propertyName, const std::vector<T>& values)
        {
            uint32_t numElements = static_cast<uint32_t>(values.size());
            write(propertyName, numElements);

            for (uint32_t i = 0; i < numElements; ++i)
            {
                write("element", values[i]);
            }
        }

        template<typename T>
        void writeValues(const char* propertyName, const std::set<T>& values)
        {
            uint32_t numElements = static_cast<uint32_t>(values.size());
            write(propertyName, numElements);

            for (const auto& v : values)
            {
                write("element", v);
            }
        }

        /// match propertyname and write value(s)
        template<typename... Args>
        void write(const char* propertyName, Args&... args)
        {
            writePropertyName(propertyName);

            // use fold expression to expand arguments and map to appropriate write method
            (write(1, &(args)), ...);

            writeEndOfLine();
        }

        void writeObject(const char* propertyName, const Object* object)
        {
            writePropertyName(propertyName);
            write(object);
        }

        /// write a value casting it to specified type i.e. output.write<uint32_t>("Value", value);
        template<typename W, typename T>
        void writeValue(const char* propertyName, T value)
        {
            W v{static_cast<W>(value)};
            write(propertyName, v);
        }

        using ObjectID = uint32_t;
        using ObjectIDMap = std::unordered_map<const vsg::Object*, ObjectID>;

        ObjectID objectID = 1;
        ObjectIDMap objectIDMap;
        ref_ptr<const Options> options;

        VsgVersion version;

        virtual bool version_less(uint32_t major, uint32_t minor, uint32_t patch, uint32_t soversion = 0) const;
        virtual bool version_greater_equal(uint32_t major, uint32_t minor, uint32_t patch, uint32_t soversion = 0) const;

    protected:
        virtual ~Output();
    };

} // namespace vsg
