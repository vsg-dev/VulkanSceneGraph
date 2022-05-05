#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Export.h>

#include <string>
#include <ostream>

#define NEW_PATH_DEFINED 1

#if defined(WIN32)
    #define WIDE_PATH 1
#else
    #define WIDE_PATH 0
#endif

#include <iostream>

namespace vsg
{

    extern VSG_DECLSPEC void copy(const std::string& src, std::wstring& dst);
    extern VSG_DECLSPEC void copy(const std::wstring& src, std::string& dst);

    inline void copy(const std::string& src, std::string& dst) { dst = src; }
    inline void copy(const std::wstring& src, std::wstring& dst) { dst = src; }
    inline void copy(const char c, std::string& dst) { dst.clear(); dst.push_back(c); }
    inline void copy(const char c, std::wstring& dst) { dst.clear(); dst.push_back(static_cast<wchar_t>(c)); }

#if NEW_PATH_DEFINED

    class VSG_DECLSPEC Path
    {
    public:

#if WIDE_PATH
        using value_type = wchar_t;
#else
        using value_type = char;
#endif

        using string_type = std::basic_string<value_type>;

        using size_type = size_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using iterator = string_type::iterator;
        using const_iterator = string_type::const_iterator;
        using pointer = value_type*;
        using const_pointer = const value_type*;

        static const size_type npos = static_cast<size_type>(-1);

        Path();
        Path(const Path& path);
        Path(const char* str);
        Path(const std::string& str);
        Path(const std::wstring& str);

        void assign(const std::string& str) { copy(str, _string); }
        void assign(const char* str) { copy(str, _string); }
        void assign(const std::wstring& str) { copy(str, _string); }

        inline string_type native(const std::string& str) const { string_type dest; copy(str, dest); return dest; }
        inline string_type native(const std::wstring& str) const { string_type dest; copy(str, dest); return dest; }
        inline string_type native(const char* str) const { string_type dest; copy(str, dest); return dest; }
        inline string_type native(const char c) const { string_type dest; copy(c, dest); return dest; }

        iterator begin() { return _string.begin(); }
        iterator end() { return _string.end(); }
        const_iterator begin() const { return _string.begin(); }
        const_iterator end() const  { return _string.end(); }

        int compare(const Path& rhs) const { return _string.compare(rhs._string); }
        int compare(size_type pos, size_type n, const Path& rhs) const { return _string.compare(pos, n, rhs._string); }

        int compare(const char* rhs) const { return _string.compare(native(rhs)); }
        int compare(size_type pos, size_type n, const char* rhs) const { return _string.compare(pos, n, native(rhs)); }

        Path& operator = (const Path& path) {if (this != &path) _string = path._string; return *this; }
        Path& operator = (const std::string& str) { assign(str); return *this; }
        Path& operator = (const char* str) { assign(str); return *this; }

        bool operator == (const Path& rhs) const { return compare(rhs) == 0; }
        bool operator != (const Path& rhs) const { return compare(rhs) != 0; }
        bool operator < (const Path& rhs) const { return compare(rhs) < 0; }

        bool operator == (const char* rhs) const { return compare(native(rhs)) == 0; }
        bool operator != (const char* rhs) const { return compare(native(rhs)) != 0; }

        bool empty() const { return _string.empty(); }
        size_type size() const { return _string.size(); }
        size_type length() const { return _string.size(); }

        inline std::string string() const { std::string dest; copy(_string, dest); return dest; }
        inline std::wstring wstring() const { std::wstring dest; copy(_string, dest); return dest; }

        inline const string_type& native() const noexcept { return _string; }
        inline operator const string_type& () const noexcept { return _string; }
        inline const value_type* c_str() const noexcept { return _string.c_str(); }

        reference operator [] (size_type pos) { return _string[pos]; }
        const_reference operator [] (size_type pos) const { return _string[pos]; }


        Path substr(size_type pos, size_type len = Path::npos) const { return Path(_string.substr(pos, len)); }

        size_type find(const Path& s, size_type pos = 0) const { return _string.find(s._string, pos); }
        size_type find(const char* s, size_type pos = 0) const { return _string.find(native(s), pos); }

        size_type find_first_of(const Path& s, size_type pos = 0) const { return _string.find_first_of(s._string, pos); }
        size_type find_first_of(const char* s, size_type pos = 0) const { return find_first_of(native(s), pos); }
        size_type find_first_of(const char c, size_type pos = 0) const { return find_first_of(native(c), pos); }

        size_type find_last_of(const Path& s, size_type pos = npos) const { return _string.find_last_of(s._string, pos); }
        size_type find_last_of(const char* s, size_type pos = npos) const { return find_last_of(native(s), pos); }
        size_type find_last_of(const char c, size_type pos = npos) const { return find_last_of(native(c), pos); }

        void append(const Path& path) { _string.append(path._string); }
        void append(char c) { _string.push_back(c); }

        Path& replace(size_type pos, size_type n, const Path& str) { _string.replace(pos, n, str._string); return *this; }
        Path& replace(size_type pos, size_type n, const std::string& str) { _string.replace(pos, n, native(str)); return *this; }
        Path& replace(size_type pos, size_type n, const std::wstring& str) { _string.replace(pos, n, native(str)); return *this; }
        Path& replace(size_type pos, size_type n, const char* str) {  _string.replace(pos, n, native(str)); return *this; }

    protected:
        string_type _string = {};

        #if WIDE_PATH
        mutable std::string _cache;
        #endif

    };

    inline Path operator + (const Path& lhs, const Path& rhs)
    {
        Path path(lhs);
        path.append(rhs);
        return path;
    }

    inline Path operator + (const Path& lhs, char rhs)
    {
        Path path(lhs);
        path.append(rhs);
        return path;
    }

    inline std::ostream& operator<<(std::ostream& output, const vsg::Path& path)
    {
        output << path.string();
        return output;
    }

    inline std::istream& operator>>(std::istream& input, vsg::Path& path)
    {
        std::string str;
        input >> str;
        path = str;
        return input;
    }

#else
    using Path = std::string;
#endif

} // namespace vsg
