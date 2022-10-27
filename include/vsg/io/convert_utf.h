#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Export.h>

#include <string>

namespace vsg
{

    /// convert from UTF8 std::string to a std::wstring
    extern VSG_DECLSPEC void convert_utf(const std::string& src, std::wstring& dst);

    /// convert from std::wstring to a UTF8 std::string
    extern VSG_DECLSPEC void convert_utf(const std::wstring& src, std::string& dst);

    inline void convert_utf(const std::string& src, std::string& dst) { dst = src; }
    inline void convert_utf(const std::wstring& src, std::wstring& dst) { dst = src; }
    inline void convert_utf(const char c, std::string& dst)
    {
        dst.clear();
        dst.push_back(c);
    }
    inline void convert_utf(const char c, std::wstring& dst)
    {
        dst.clear();
        dst.push_back(static_cast<wchar_t>(c));
    }
    inline void convert_utf(const wchar_t c, std::string& dst)
    {
        std::wstring src;
        src.push_back(c);
        convert_utf(src, dst);
    }
    inline void convert_utf(const wchar_t c, std::wstring& dst)
    {
        dst.clear();
        dst.push_back(c);
    }

    template<typename T>
    T convert_utf(const std::string& src)
    {
        T dst;
        convert_utf(src, dst);
        return dst;
    }

    template<typename T>
    T convert_utf(const char* src)
    {
        T dst;
        convert_utf(src, dst);
        return dst;
    }

    template<typename T>
    T convert_utf(const char c)
    {
        T dst;
        convert_utf(c, dst);
        return dst;
    }

    template<typename T>
    T convert_utf(const std::wstring& src)
    {
        T dst;
        convert_utf(src, dst);
        return dst;
    }

    template<typename T>
    T convert_utf(const wchar_t* src)
    {
        T dst;
        convert_utf(src, dst);
        return dst;
    }

    template<typename T>
    T convert_utf(const wchar_t c)
    {
        T dst;
        convert_utf(c, dst);
        return dst;
    }

} // namespace vsg
