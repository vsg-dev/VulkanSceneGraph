/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/io/Path.h>

using namespace vsg;

Path::Path()
{
}

Path::Path(const Path& path) :
    _string(path._string)
{
}

Path::Path(const char* str)
{
    assign(str);
}

Path::Path(const std::string& str)
{
    assign(str);
}

Path::Path(const wchar_t* str)
{
    assign(str);
}

Path::Path(const std::wstring& str)
{
    assign(str);
}

Path& Path::assign(const Path& path)
{
    if (&path != this) _string = path._string;
    return *this;
}

Path& Path::assign(const std::string& str)
{
    convert_utf(str, _string);
    return *this;
}

Path& Path::assign(const char* str)
{
    convert_utf(str, _string);
    return *this;
}

Path& Path::assign(const std::wstring& str)
{
    convert_utf(str, _string);
    return *this;
}

Path& Path::assign(const wchar_t* str)
{
    convert_utf(str, _string);
    return *this;
}

Path& Path::replace(size_type pos, size_type n, const Path& str)
{
    _string.replace(pos, n, str._string);
    return *this;
}
Path& Path::replace(size_type pos, size_type n, const std::string& str)
{
    _string.replace(pos, n, convert_utf<string_type>(str));
    return *this;
}
Path& Path::replace(size_type pos, size_type n, const std::wstring& str)
{
    _string.replace(pos, n, convert_utf<string_type>(str));
    return *this;
}
Path& Path::replace(size_type pos, size_type n, const char* str)
{
    _string.replace(pos, n, convert_utf<string_type>(str));
    return *this;
}
Path& Path::replace(size_type pos, size_type n, const wchar_t* str)
{
    _string.replace(pos, n, convert_utf<string_type>(str));
    return *this;
}

Path& Path::append(const Path& right)
{
    if (empty())
    {
        return assign(right);
    }

    if (right.empty())
    {
        return *this;
    }

    auto lastChar = _string[_string.size() - 1];
    if (lastChar == preferred_separator)
    {
        concat(right._string);
    }
    else if (lastChar == alternate_separator)
    {
        _string.erase(_string.size() - 1, 1);
        _string.push_back(preferred_separator);
        _string.append(right._string);
    }
    else // lastChar != a delimiter
    {
        _string.push_back(preferred_separator);
        _string.append(right._string);
    }

    return *this;
}

Path& Path::erase(size_t pos, size_t len)
{
    _string.erase(pos, len);
    return *this;
}

FileType Path::type() const
{
    return vsg::fileType(*this);
}
