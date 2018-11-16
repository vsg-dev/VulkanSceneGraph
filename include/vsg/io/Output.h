#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Object.h>

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>

namespace vsg
{

    class Output
    {
    public:
        // write property name if appropriate for format
        virtual void writePropertyName(const char* propertyName) = 0;

        // write contiguous array of value(s)
        virtual void write(size_t num, const int8_t* values) = 0;
        virtual void write(size_t num, const uint8_t* value) = 0;
        virtual void write(size_t num, const int16_t* value) = 0;
        virtual void write(size_t num, const uint16_t* value) = 0;
        virtual void write(size_t num, const int32_t* value) = 0;
        virtual void write(size_t num, const uint32_t* value) = 0;
        virtual void write(size_t num, const int64_t* value) = 0;
        virtual void write(size_t num, const uint64_t* value) = 0;
        virtual void write(size_t num, const float* value) = 0;
        virtual void write(size_t num, const double* value) = 0;
        virtual void write(size_t num, const std::string* value) = 0;

        // write object
        virtual void write(const Object* object) = 0;

        // map char to int8_t
        void write(size_t num, const char* value) { write(num, reinterpret_cast<const int8_t*>(value)); }
        void write(size_t num, const bool* value) { write(num, reinterpret_cast<const int8_t*>(value)); }

        // vec/mat versions of write methods
        void write(size_t num, const vec2* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const dvec2* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const vec3* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const dvec3* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const vec4* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const dvec4* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const mat4* value) { write(num * value->size(), value->data()); }
        void write(size_t num, const dmat4* value) { write(num * value->size(), value->data()); }

        // match propertyname and write value(s)
        template<typename... Args>
        void write(const char* propertyName, Args&... args)
        {
            writePropertyName(propertyName);

            // use fold expression to expand arugments and map to appropriate write method
            (write(1, &(args)), ...);
        }

        void writeObject(const char* propertyName, const Object* object)
        {
            writePropertyName(propertyName);
            write(object);
        }

        /// write a value casting it specified type i.e. output.write<uint32_t>("Value", value);
        template<typename W, typename T>
        void writeValue(const char* propertyName, T value)
        {
            W v{static_cast<W>(value)};
            write(propertyName, v);
        }
    };

} // namespace vsg
