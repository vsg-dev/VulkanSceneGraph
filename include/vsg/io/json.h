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

    /// JSON parser based on spec: https://www.json.org/json-en.html
    struct VSG_DECLSPEC JSONParser
    {
        std::string buffer;
        std::size_t pos = 0;
        mem_stream mstr;

        JSONParser();

        /// Schema base class to provides a mechanism for customizing the json parsing to handle
        /// mapping between json schema's and user data/scene graph objects
        struct Schema
        {
            // array elements [ value, value.. ]
            virtual void read_array(JSONParser& parser);
            virtual void read_object(JSONParser& parser);
            virtual void read_string(JSONParser& parser);
            virtual void read_number(JSONParser& parser, std::istream& input);
            virtual void read_bool(JSONParser& parser, bool value);
            virtual void read_null(JSONParser& parser);

            // object properties { name, value; ... }
            virtual void read_array(JSONParser& parser, const std::string_view& name);
            virtual void read_object(JSONParser& parser, const std::string_view& name);
            virtual void read_string(JSONParser& parser, const std::string_view& name);
            virtual void read_number(JSONParser& parser, const std::string_view& name, std::istream& input);
            virtual void read_bool(JSONParser& parser, const std::string_view& name, bool value);
            virtual void read_null(JSONParser& parser, const std::string_view& name);
        };

        bool read_string(std::string& value);
        void read_object(Schema& schema);
        void read_array(Schema& schema);

        std::size_t lineNumberAtPosition(std::size_t postion) const;
        std::string_view lineEnclosingPosition(std::size_t postion) const;

        Logger::Level level = Logger::LOGGER_WARN;

        template<typename... Args>
        void warning(Args&&... args)
        {
            log(level, "Parsing error at line ", lineNumberAtPosition(pos), " [ ", lineEnclosingPosition(pos), " ]. ", std::forward<Args>(args)...);
        }

        inline bool white_space(char c) const
        {
            return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
        }
    };
    VSG_type_name(vsg::JSONParser);

    /// Default support for mapping standard JSON types directly to VSG.
    /// JSON objects are mapped to Object metadata.
    /// JSON arrays to Objects.
    /// string, number and bool are mapped to stringValue, doubleValue and boolValue.
    struct VSG_DECLSPEC JSONtoMetaDataSchema : public JSONParser::Schema
    {
        // object created when parsing JSON file
        ref_ptr<Object> object;
        ref_ptr<Objects> objects;

        void addToArray(ref_ptr<Object> in_object);
        void addToObject(const std::string_view& name, ref_ptr<Object> in_object);

        // array elements [ value, value.. ]
        void read_array(JSONParser& parser) override;
        void read_object(JSONParser& parser) override;
        void read_string(JSONParser& parser) override;
        void read_number(JSONParser& parser, std::istream& input) override;
        void read_bool(JSONParser& parser, bool value) override;
        void read_null(JSONParser& parser) override;

        // object properties { name, value; ... }
        void read_array(JSONParser& parser, const std::string_view& name) override;
        void read_object(JSONParser& parser, const std::string_view& name) override;
        void read_string(JSONParser& parser, const std::string_view& name) override;
        void read_number(JSONParser& parser, const std::string_view& name, std::istream& input) override;
        void read_bool(JSONParser& parser, const std::string_view& name, bool value) override;
        void read_null(JSONParser& parser, const std::string_view& name) override;
    };
    VSG_type_name(vsg::JSONtoMetaDataSchema);

    /// json ReaderWriter
    class json : public Inherit<ReaderWriter, json>
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
