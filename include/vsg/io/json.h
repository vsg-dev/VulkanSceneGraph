#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2025 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shimages be included in images
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Objects.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/io/mem_stream.h>

namespace vsg
{

    /// json ReaderWriter
    class VSG_DECLSPEC json : public Inherit<ReaderWriter, json>
    {
    public:
        json();

        ref_ptr<Object> read(const Path&, ref_ptr<const Options>) const override;
        ref_ptr<Object> read(std::istream&, ref_ptr<const Options>) const override;
        ref_ptr<Object> read(const uint8_t* ptr, size_t size, ref_ptr<const Options> options = {}) const override;

        ref_ptr<Object> _read(std::istream&, ref_ptr<const Options>) const;

        bool supportedExtension(const Path& ext) const;

        bool getFeatures(Features& features) const override;
    };
    VSG_type_name(vsg::json);

} // namespace vsg
