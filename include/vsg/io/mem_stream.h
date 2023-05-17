#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Export.h>

#include <cstdint>
#include <istream>

namespace vsg
{

    /// Input stream that enables reading from a read only block of memory
    /// Like std::string_view the memory referenced by the mem_stream has been kept in memory for the duration of the mem_stream existence.
    class VSG_DECLSPEC mem_stream : public std::istream
    {
    public:
        mem_stream(const uint8_t* ptr, size_t length);
        mem_stream(const std::string& str, std::string::size_type pos, std::string::size_type length);
        explicit mem_stream(const std::string_view& sv);

        /// set the mem_stream to memory block
        void set(const uint8_t* ptr, size_t length);

        /// set the mem_stream to string_view
        void set(const std::string_view& sv) { set(reinterpret_cast<const uint8_t*>(sv.data()), sv.size()); }

        /// set the mem_stream to portion of string
        void set(const std::string& str, std::string::size_type pos, std::string::size_type length) { set(reinterpret_cast<const uint8_t*>(&(str[pos])), length); }

    private:
        struct mem_buffer : public std::streambuf
        {
            mem_buffer(const uint8_t* ptr, size_t length);

            inline void set(const uint8_t* ptr, size_t length)
            {
                setg((char*)(ptr), (char*)(ptr), (char*)(ptr) + length);
            }
        };

        mem_buffer _buffer;
    };

} // namespace vsg
