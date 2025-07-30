/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/mem_stream.h>

using namespace vsg;

mem_stream::mem_stream(const uint8_t* ptr, size_t length) :
    std::istream(&_buffer),
    _buffer(ptr, length)
{
    rdbuf(&_buffer);
}

mem_stream::mem_stream(const std::string_view& sv) :
    mem_stream(reinterpret_cast<const uint8_t*>(sv.data()), sv.size())
{
}

mem_stream::mem_stream(const std::string& str, std::string::size_type pos, std::string::size_type length) :
    mem_stream(reinterpret_cast<const uint8_t*>(&(str[pos])), length)
{
}

void mem_stream::set(const uint8_t* ptr, size_t length)
{
    _buffer.set(ptr, length);
    clear();
}

mem_stream::mem_buffer::mem_buffer(const uint8_t* ptr, size_t length)
{
    setg((char*)(ptr), (char*)(ptr), (char*)(ptr) + length);
}

std::streambuf::pos_type mem_stream::mem_buffer::seekoff(std::streambuf::off_type offset, std::ios_base::seekdir dir, std::ios_base::openmode /*mode*/)
{
    if (dir == std::ios_base::beg)
    {
        setg(eback(), eback() + offset, egptr());
    }
    else if (dir == std::ios_base::end)
    {
        setg(eback(), egptr() - offset, egptr());
    }
    else // dir == std::ios_base::cur
    {
        setg(eback(), gptr() + offset, egptr());
    }

    return pos_type(gptr() - eback());
}

std::streambuf::pos_type mem_stream::mem_buffer::seekpos(std::streambuf::pos_type pos, std::ios_base::openmode /*mode*/)
{
    setg(eback(), eback() + pos, egptr());
    return pos_type(pos);
}
