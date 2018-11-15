#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Object.h>

namespace vsg
{

    class Input
    {
    public:
        /// return true if property name matches the next token in the stream, or if propery names are not required for format
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
        virtual void read(size_t num, std::string* value) = 0;

        // read object
        virtual ref_ptr<Object> read() = 0;

        // map char to int8_t
        void read(size_t num, char* value) { read(num, reinterpret_cast<int8_t*>(value)); }
        void read(size_t num, bool* value) { read(num, reinterpret_cast<int8_t*>(value)); }

        // vec/mat versions of read methods
        void read(size_t num, vec2* value) { read(num * value->size(), value->data()); }
        void read(size_t num, dvec2* value) { read(num * value->size(), value->data()); }
        void read(size_t num, vec3* value) { read(num * value->size(), value->data()); }
        void read(size_t num, dvec3* value) { read(num * value->size(), value->data()); }
        void read(size_t num, vec4* value) { read(num * value->size(), value->data()); }
        void read(size_t num, dvec4* value) { read(num * value->size(), value->data()); }
        void read(size_t num, mat4* value) { read(num * value->size(), value->data()); }
        void read(size_t num, dmat4* value) { read(num * value->size(), value->data()); }

        // match property name and read value(s)
        template<typename... Args>
        void read(const char* propertyName, Args&... args)
        {
            if (!matchPropertyName(propertyName)) return;

            // use fold expression to expand arugments and map to appropriate read method
            (read(1, &(args)), ...);
        }

        // read object of a particular type
        ref_ptr<Object> readObject(const char* propertyName)
        {
            if (!matchPropertyName(propertyName)) return ref_ptr<Object>();

            return read();
        }

        // read object of a particular type
        template<class T>
        ref_ptr<T> readObject(const char* propertyName)
        {
            if (!matchPropertyName(propertyName)) return ref_ptr<T>();

            ref_ptr<Object> object = read();
            return ref_ptr<T>(dynamic_cast<T*>(object.get()));
        }

        // read a value of particular type
        template<typename T>
        T readValue(const char* propertyName)
        {
            T value{};
            read(propertyName, value);
            return value;
        }
    };

} // namespace vsg
