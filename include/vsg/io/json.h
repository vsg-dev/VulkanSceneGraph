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

#include <vsg/io/ReaderWriter.h>
#include <vsg/io/mem_stream.h>

namespace vsg
{

    /// JSON parser based on spec: https://www.json.org/json-en.html
    struct JSONParser
    {
        std::string buffer;
        std::size_t pos = 0;
        vsg::mem_stream mstr;
        vsg::indentation indent;

        JSONParser();

        inline bool white_space(char c) const
        {
            return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
        }

        bool read_string(std::string& value);

        vsg::ref_ptr<vsg::Object> read_array();
        vsg::ref_ptr<vsg::Object> read_object();
    };
    VSG_type_name(vsg::JSONParser);

    /// json ReaderWriter
    class json : public vsg::Inherit<vsg::ReaderWriter, json>
    {
    public:
        json();

        vsg::ref_ptr<vsg::Object> read(const vsg::Path&, vsg::ref_ptr<const vsg::Options>) const override;
        vsg::ref_ptr<vsg::Object> read(std::istream&, vsg::ref_ptr<const vsg::Options>) const override;
        vsg::ref_ptr<vsg::Object> read(const uint8_t* ptr, size_t size, vsg::ref_ptr<const vsg::Options> options = {}) const override;

        vsg::ref_ptr<vsg::Object> _read(std::istream&, vsg::ref_ptr<const vsg::Options>) const;

        bool supportedExtension(const vsg::Path& ext) const;

        bool getFeatures(Features& features) const override;
    };
    VSG_type_name(vsg::json);

} // namespace vsg
