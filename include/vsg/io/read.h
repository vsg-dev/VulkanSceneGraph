#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <istream>
#include <vsg/core/Inherit.h>
#include <vsg/io/FileSystem.h>
#include <vsg/io/Options.h>

namespace vsg
{

    /** convenience method for reading objects from file.*/
    extern VSG_DECLSPEC ref_ptr<Object> read(const Path& filename, ref_ptr<const Options> options = {});

    /** convenience method for reading objects from files.*/
    extern VSG_DECLSPEC PathObjects read(const Paths& filenames, ref_ptr<const Options> options = {});

    /** convenience method for reading objects from stream.*/
    extern VSG_DECLSPEC ref_ptr<Object> read(std::istream& fin, ref_ptr<const Options> options = {});

    /** convenience method for reading objects from memory.*/
    extern VSG_DECLSPEC ref_ptr<Object> read(const uint8_t* ptr, size_t size, ref_ptr<const Options> options = {});

    /** convenience method for reading file with cast to specified type.*/
    template<class T>
    ref_ptr<T> read_cast(const Path& filename, ref_ptr<const Options> options = {})
    {
        auto object = read(filename, options);
        return vsg::ref_ptr<T>(dynamic_cast<T*>(object.get()));
    }

    /** convenience method for reading stream with cast to specified type.*/
    template<class T>
    ref_ptr<T> read_cast(std::istream& fin, ref_ptr<const Options> options = {})
    {
        auto object = read(fin, options);
        return vsg::ref_ptr<T>(dynamic_cast<T*>(object.get()));
    }

    /** convenience method for reading memory with cast to specified type.*/
    template<class T>
    ref_ptr<T> read_cast(const uint8_t* ptr, size_t size, ref_ptr<const Options> options = {})
    {
        auto object = read(ptr, size, options);
        return vsg::ref_ptr<T>(dynamic_cast<T*>(object.get()));
    }

} // namespace vsg
