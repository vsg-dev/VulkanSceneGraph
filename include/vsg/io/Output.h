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

    class Output
    {
    public:
        // write single values
        virtual void write(const char* propertyName, int8_t value) = 0;
        virtual void write(const char* propertyName, uint8_t value) = 0;
        virtual void write(const char* propertyName, int16_t value) = 0;
        virtual void write(const char* propertyName, uint16_t value) = 0;
        virtual void write(const char* propertyName, int32_t value) = 0;
        virtual void write(const char* propertyName, uint32_t value) = 0;
        virtual void write(const char* propertyName, int64_t value) = 0;
        virtual void write(const char* propertyName, uint64_t value) = 0;
        virtual void write(const char* propertyName, float value) = 0;
        virtual void write(const char* propertyName, double value) = 0;

#if 0
        // write contiguous array of values
        virtual void write(const char* propertyName, size_t num, const int8_t* values) = 0;
        virtual void write(const char* propertyName, size_t num, const uint8_t& value) = 0;
        virtual void write(const char* propertyName, size_t num, const int16_t& value) = 0;
        virtual void write(const char* propertyName, size_t num, const uint16_t& value) = 0;
        virtual void write(const char* propertyName, size_t num, const int32_t& value) = 0;
        virtual void write(const char* propertyName, size_t num, const uint32_t& value) = 0;
        virtual void write(const char* propertyName, size_t num, const int64_t& value) = 0;
        virtual void write(const char* propertyName, size_t num, const uint64_t& value) = 0;
        virtual void write(const char* propertyName, size_t num, const float* value) = 0;
        virtual void write(const char* propertyName, size_t num, const double* value) = 0;
#endif

        // write object
        virtual void write(const char* propertyName, const Object* object) = 0;

        template<typename W, typename T>
        void write(const char* propertyName, W value)
        {
            W v{static_cast<W>(value)};
            write(propertyName, v);
        }
    };

} // namespace vsg
