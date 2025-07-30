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
#include <vsg/io/stream.h>

#include <list>

namespace vsg
{

    /// JSON parser based on spec: https://www.json.org/json-en.html
    struct VSG_DECLSPEC JSONParser : public Inherit<Object, JSONParser>
    {
        ref_ptr<const Options> options;
        std::string buffer;
        std::size_t pos = 0;
        mem_stream mstr;

        JSONParser();

        /// Schema base class to provides a mechanism for customizing the json parsing to handle
        /// mapping between json schema's and user data/scene graph objects
        struct VSG_DECLSPEC Schema : public Inherit<Object, Schema>
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

        bool read_uri(std::string& value, ref_ptr<Object>& object);
        bool read_string_view(std::string_view& value);
        bool read_string(std::string& value);
        void read_object(Schema& schema);
        void read_array(Schema& schema);

        std::pair<std::size_t, std::size_t> lineAndColumnAtPosition(std::size_t position) const;
        std::string_view lineEnclosingPosition(std::size_t position) const;

        std::list<std::string> warnings;

        template<typename... Args>
        void warning(Args&&... args)
        {
            auto [line, column] = lineAndColumnAtPosition(pos);
            warnings.push_back(vsg::make_string("Parsing error at [", line, ":", column, "], pos = ", pos, " [ ", lineEnclosingPosition(pos), " ]. ", std::forward<Args>(args)...));
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
    struct VSG_DECLSPEC JSONtoMetaDataSchema : public Inherit<JSONParser::Schema, JSONtoMetaDataSchema>
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

    /// class for reading array of strings.
    struct StringsSchema : public Inherit<JSONParser::Schema, StringsSchema>
    {
        std::vector<std::string> values;
        void read_string(JSONParser& parser) override
        {
            std::string value;
            parser.read_string(value);
            values.push_back(value);
        }
    };

    /// Template class for reading an array of numeric values
    template<typename T>
    struct ValuesSchema : public Inherit<JSONParser::Schema, ValuesSchema<T>>
    {
        std::vector<T> values;
        void read_number(vsg::JSONParser& parser, std::istream& input) override
        {
            T value;
            input >> value;
            values.push_back(value);
        }
    };

    /// Template class for reading an array of objects
    template<typename T>
    struct ObjectsSchema : public Inherit<JSONParser::Schema, ObjectsSchema<T>>
    {
        std::vector<ref_ptr<T>> values;

        void read_object(vsg::JSONParser& parser) override
        {
            values.push_back(T::create());
            parser.read_object(*values.back());
        }

        void report(vsg::LogOutput& output)
        {
            for (auto& value : values) value->report(output);
        }
    };

} // namespace vsg
