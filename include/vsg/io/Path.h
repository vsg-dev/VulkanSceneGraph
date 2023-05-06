#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/convert_utf.h>

#include <string>

namespace vsg
{

    enum FileType
    {
        FILE_NOT_FOUND = 0,
        REGULAR_FILE,
        DIRECTORY
    };

    /// Class for managing paths/filename with full support for wide and single wide path strings.
    /// Similar in role and features to std::filesystem::path, but is able to work on older compilers.
    class VSG_DECLSPEC Path
    {
    public:
#if defined(_MSC_VER) || defined(__MINGW32__)
        using value_type = wchar_t;
        static constexpr value_type windows_separator = L'\\';
        static constexpr value_type posix_separator = L'/';
        static constexpr value_type preferred_separator = windows_separator;
        static constexpr value_type alternate_separator = posix_separator;
        static constexpr const value_type* separators = L"/\\";
#else
        using value_type = char;
        static constexpr value_type windows_separator = '\\';
        static constexpr value_type posix_separator = '/';
        static constexpr value_type preferred_separator = posix_separator;
        static constexpr value_type alternate_separator = windows_separator;
        static constexpr const value_type* separators = "/\\";
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
        Path(const std::string& str);
        Path(const char* str);
        Path(const std::wstring& str);
        Path(const wchar_t* str);

        iterator begin() { return _string.begin(); }
        iterator end() { return _string.end(); }
        const_iterator begin() const { return _string.begin(); }
        const_iterator end() const { return _string.end(); }

        Path& assign(const Path& path);
        Path& assign(const std::string& str);
        Path& assign(const char* str);
        Path& assign(const std::wstring& str);
        Path& assign(const wchar_t* str);

        Path& operator=(const Path& path) { return assign(path); }
        Path& operator=(const std::string& str) { return assign(str); }
        Path& operator=(const char* str) { return assign(str); }
        Path& operator=(const std::wstring& str) { return assign(str); }
        Path& operator=(const wchar_t* str) { return assign(str); }

        int compare(const Path& rhs) const { return _string.compare(rhs._string); }
        int compare(size_type pos, size_type n, const Path& rhs) const { return _string.compare(pos, n, rhs._string); }

        int compare(const char* rhs) const { return _string.compare(convert_utf<string_type>(rhs)); }
        int compare(const wchar_t* rhs) const { return _string.compare(convert_utf<string_type>(rhs)); }
        int compare(size_type pos, size_type n, const char* rhs) const { return _string.compare(pos, n, convert_utf<string_type>(rhs)); }
        int compare(size_type pos, size_type n, const wchar_t* rhs) const { return _string.compare(pos, n, convert_utf<string_type>(rhs)); }

        bool operator==(const Path& rhs) const { return compare(rhs) == 0; }
        bool operator!=(const Path& rhs) const { return compare(rhs) != 0; }
        bool operator<(const Path& rhs) const { return compare(rhs) < 0; }

        bool operator==(const char* rhs) const { return compare(convert_utf<string_type>(rhs)) == 0; }
        bool operator!=(const char* rhs) const { return compare(convert_utf<string_type>(rhs)) != 0; }

        bool operator==(const wchar_t* rhs) const { return compare(convert_utf<string_type>(rhs)) == 0; }
        bool operator!=(const wchar_t* rhs) const { return compare(convert_utf<string_type>(rhs)) != 0; }

        explicit operator bool() const noexcept { return !_string.empty(); }
        bool empty() const { return _string.empty(); }
        size_type size() const { return _string.size(); }
        size_type length() const { return _string.size(); }

        inline std::string string() const
        {
            std::string dest;
            convert_utf(_string, dest);
            return dest;
        }
        inline std::wstring wstring() const
        {
            std::wstring dest;
            convert_utf(_string, dest);
            return dest;
        }

        inline const string_type& native() const noexcept { return _string; }
        inline operator const string_type&() const noexcept { return _string; }
        inline const value_type* c_str() const noexcept { return _string.c_str(); }
#if defined(__MINGW32__)
        inline operator const value_type*() const noexcept
        {
            return _string.c_str();
        }
#endif

        reference operator[](size_type pos)
        {
            return _string[pos];
        }
        const_reference operator[](size_type pos) const { return _string[pos]; }

        void clear() noexcept { _string.clear(); }
        void swap(Path& rhs) noexcept { return _string.swap(rhs._string); }

        /// directly add to end of path without a path separator
        Path& concat(const Path& path)
        {
            _string.append(path._string);
            return *this;
        }

        /// directly add to end of path without a path separator
        Path& concat(char c)
        {
            _string.push_back(c);
            return *this;
        }

        /// directly add to end of path without a path separator
        Path& operator+=(const Path& path) { return concat(path); }

        /// add to end of path with path separator
        Path& append(const Path& path);

        /// add to end of path with path separator
        Path& operator/=(const Path& path) { return append(path); }

        Path substr(size_type pos, size_type len = Path::npos) const { return Path(_string.substr(pos, len)); }

        size_type find(const Path& s, size_type pos = 0) const { return _string.find(s._string, pos); }
        size_type find(const char* s, size_type pos = 0) const { return _string.find(convert_utf<string_type>(s), pos); }
        size_type find(const wchar_t* s, size_type pos = 0) const { return _string.find(convert_utf<string_type>(s), pos); }

        size_type find_first_of(const Path& s, size_type pos = 0) const { return _string.find_first_of(s._string, pos); }
        size_type find_first_of(const char* s, size_type pos = 0) const { return find_first_of(convert_utf<string_type>(s), pos); }
        size_type find_first_of(const char c, size_type pos = 0) const { return find_first_of(convert_utf<string_type>(c), pos); }
        size_type find_first_of(const wchar_t* s, size_type pos = 0) const { return find_first_of(convert_utf<string_type>(s), pos); }
        size_type find_first_of(const wchar_t c, size_type pos = 0) const { return find_first_of(convert_utf<string_type>(c), pos); }

        size_type find_last_of(const Path& s, size_type pos = npos) const { return _string.find_last_of(s._string, pos); }
        size_type find_last_of(const char* s, size_type pos = npos) const { return find_last_of(convert_utf<string_type>(s), pos); }
        size_type find_last_of(const char c, size_type pos = npos) const { return find_last_of(convert_utf<string_type>(c), pos); }
        size_type find_last_of(const wchar_t* s, size_type pos = npos) const { return find_last_of(convert_utf<string_type>(s), pos); }
        size_type find_last_of(const wchar_t c, size_type pos = npos) const { return find_last_of(convert_utf<string_type>(c), pos); }

        Path& replace(size_type pos, size_type n, const Path& str);
        Path& replace(size_type pos, size_type n, const std::string& str);
        Path& replace(size_type pos, size_type n, const std::wstring& str);
        Path& replace(size_type pos, size_type n, const char* str);
        Path& replace(size_type pos, size_type n, const wchar_t* str);

        Path& erase(size_t pos = 0, size_t len = Path::npos);

        FileType type() const;

        Path lexically_normal() const;

    protected:
        string_type _string;
    };
    VSG_type_name(vsg::Path);

    /// directly join two paths without a path separator
    inline Path operator+(const Path& lhs, const Path& rhs)
    {
        Path path(lhs);
        return path.concat(rhs);
    }

    /// join two paths with a path separator between
    inline Path operator/(const Path& lhs, const Path& rhs)
    {
        Path path(lhs);
        return path /= rhs;
    }

    using Paths = std::vector<Path>;
    using PathObjects = std::map<Path, ref_ptr<Object>>;

    /// return path stripped of the filename or final path component.
    extern VSG_DECLSPEC Path filePath(const Path& path);

    /// return file extension include the . prefix, i.e. vsg::fileExtension("file.vsgt") returns .vsgt
    extern VSG_DECLSPEC Path fileExtension(const Path& path);

    /// return lower case file extension include the . prefix, i.e. vsg::fileExtension("file.VSGT") returns .vsgt
    /// By default prunes extras such as REST strings at the end of the extensions, uses ? as the deliminator for REST additions i.e. ".jpeg?g=42" becomes ".jpeg"
    extern VSG_DECLSPEC Path lowerCaseFileExtension(const Path& path, bool pruneExtras = true);

    /// return the filename stripped of any paths and extensions, i.e vsg::simpleFilname("path/file.vsgb") returns file
    extern VSG_DECLSPEC Path simpleFilename(const Path& path);

    /// return true if the path equals ., .. or has a trailing \.. \.., /.. or /....
    extern VSG_DECLSPEC bool trailingRelativePath(const Path& path);

    /// return the path minus the extension, i.e. vsg::removeExtension("path/file.png") return path/file
    extern VSG_DECLSPEC Path removeExtension(const Path& path);

} // namespace vsg
