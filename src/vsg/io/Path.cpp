/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// copy between UTF9 <-> UF16
//
// https://en.wikipedia.org/wiki/UTF-8
// https://en.wikipedia.org/wiki/UTF-16
// OpenSceneGraph/src/osgText/String.cpp
void vsg::copy(const std::string& src, std::wstring& dst)
{
    dst.clear();
    std::cout<<"TODO convert from utf8 to utf16"<<std::endl;
    for(auto itr = src.begin(); itr != src.end();)
    {
        uint32_t c0 = *itr++;
        dst.push_back(static_cast<wchar_t>(c0));
    }
}

void vsg::copy(const std::wstring& src, std::string& dst)
{
    std::cout<<"TODO convert from utf16 to utf8"<<std::endl;
    for(auto itr = src.begin(); itr != src.end();)
    {
        uint32_t c0 = *itr++;
        dst.push_back(static_cast<char>(c0));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Path
//
// https://en.cppreference.com/w/cpp/filesystem/path
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

Path::Path(const std::wstring& str)
{
    assign(str);
}
