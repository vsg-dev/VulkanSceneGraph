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
        // read single values
        virtual void read(int8_t& value) = 0;
        virtual void read(uint8_t& value) = 0;
        virtual void read(int16_t& value) = 0;
        virtual void read(uint16_t& value) = 0;
        virtual void read(int32_t& value) = 0;
        virtual void read(uint32_t& value) = 0;
        virtual void read(int64_t& value) = 0;
        virtual void read(uint64_t& value) = 0;
        virtual void read(float& value) = 0;
        virtual void read(double& value) = 0;


        // read contiguous array of values
        virtual void read(size_t num, int8_t* values) = 0;
        virtual void read(size_t num, uint8_t& value) = 0;
        virtual void read(size_t num, int16_t& value) = 0;
        virtual void read(size_t num, uint16_t& value) = 0;
        virtual void read(size_t num, int32_t& value) = 0;
        virtual void read(size_t num, uint32_t& value) = 0;
        virtual void read(size_t num, int64_t& value) = 0;
        virtual void read(size_t num, uint64_t& value) = 0;
        virtual void read(size_t num, float& value) = 0;
        virtual void read(size_t num, double& value) = 0;

        // read object
        virtual ref_ptr<Object> readObject() = 0;

        // read object of a particular type
        template<class T>
        ref_ptr<T> readObject()
        {
            ref_ptr<Object> object = readObject();
            return ref_ptr<T>(dynamic_cast<T*>(object.get()));
        }

        template<typename T>
        T readValue()
        {
            T value{};
            read(value);
            return value;
        }
    };

}
