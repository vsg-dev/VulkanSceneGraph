/* <editor-fold desc="MIT License">

Copyright(c) 2025 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Objects.h>
#include <vsg/core/Value.h>
#include <vsg/io/JSONParser.h>
#include <vsg/io/Path.h>
#include <vsg/io/mem_stream.h>
#include <vsg/io/read.h>

#include <fstream>

using namespace vsg;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// JSONParser::Schema
//
void JSONParser::Schema::read_array(JSONParser&)
{
}

void JSONParser::Schema::read_object(JSONParser&)
{
}

void JSONParser::Schema::read_string(JSONParser&)
{
}

void JSONParser::Schema::read_number(JSONParser&, std::istream&)
{
}

void JSONParser::Schema::read_bool(JSONParser&, bool)
{
}

void JSONParser::Schema::read_null(JSONParser&)
{
}

void JSONParser::Schema::read_array(JSONParser&, const std::string_view&)
{
}

void JSONParser::Schema::read_object(JSONParser&, const std::string_view&)
{
}

void JSONParser::Schema::read_string(JSONParser&, const std::string_view&)
{
}

void JSONParser::Schema::read_number(JSONParser&, const std::string_view&, std::istream&)
{
}

void JSONParser::Schema::read_bool(JSONParser&, const std::string_view&, bool)
{
}

void JSONParser::Schema::read_null(JSONParser&, const std::string_view&)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// JSONtoMetaDataSchema
//

void JSONtoMetaDataSchema::addToArray(ref_ptr<Object> in_object)
{
    if (!in_object) return;

    if (!objects) objects = Objects::create();
    objects->addChild(in_object);
}

void JSONtoMetaDataSchema::addToObject(const std::string_view& name, ref_ptr<Object> in_object)
{
    if (!in_object) return;

    if (!object) object = Object::create();
    object->setObject(std::string(name), in_object);
}

void JSONtoMetaDataSchema::read_array(JSONParser& parser)
{
    JSONtoMetaDataSchema nested;
    parser.read_array(nested);

    addToArray(nested.objects);
}

void JSONtoMetaDataSchema::read_object(JSONParser& parser)
{
    JSONtoMetaDataSchema nested;
    parser.read_object(nested);

    addToArray(nested.object);
}

void JSONtoMetaDataSchema::read_string(JSONParser& parser)
{
    std::string value;
    parser.read_string(value);

    addToArray(stringValue::create(value));
}

void JSONtoMetaDataSchema::read_number(JSONParser&, std::istream& input)
{
    double value;
    input >> value;

    addToArray(doubleValue::create(value));
}

void JSONtoMetaDataSchema::read_bool(JSONParser&, bool value)
{
    addToArray(boolValue::create(value));
}

void JSONtoMetaDataSchema::read_null(JSONParser&)
{
}

void JSONtoMetaDataSchema::read_array(JSONParser& parser, const std::string_view& name)
{
    JSONtoMetaDataSchema nested;
    parser.read_array(nested);

    addToObject(name, nested.objects);
}

void JSONtoMetaDataSchema::read_object(JSONParser& parser, const std::string_view& name)
{
    JSONtoMetaDataSchema nested;
    parser.read_object(nested);

    addToObject(name, nested.object);
}

void JSONtoMetaDataSchema::read_string(JSONParser& parser, const std::string_view& name)
{
    std::string value;
    parser.read_string(value);

    addToObject(name, stringValue::create(value));
}

void JSONtoMetaDataSchema::read_number(JSONParser&, const std::string_view& name, std::istream& input)
{
    double value;
    input >> value;

    addToObject(name, doubleValue::create(value));
}

void JSONtoMetaDataSchema::read_bool(JSONParser&, const std::string_view& name, bool value)
{
    addToObject(name, boolValue::create(value));
}

void JSONtoMetaDataSchema::read_null(JSONParser&, const std::string_view&)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//
// json parser
//
JSONParser::JSONParser() :
    mstr(nullptr, 0)
{
}

bool JSONParser::read_uri(std::string& value, ref_ptr<Object>& object)
{
    if (buffer[pos] != '"') return false;

    // read string
    auto end_of_value = buffer.find('"', pos + 1);
    if (end_of_value == std::string::npos) return false;

    auto size_of_value = end_of_value - pos - 1;
    if (size_of_value >= 5)
    {
        if (buffer.compare(pos + 1, 5, "data:") == 0)
        {
            vsg::info("We have a data URI");

            auto semicolon = buffer.find(';', pos + 6);

            std::string memeType = buffer.substr(pos + 6, semicolon - pos - 6);

            vsg::info("memeType = ", memeType);

            auto comma = buffer.find(',', semicolon + 1);

            std::string encoding = buffer.substr(semicolon + 1, comma - semicolon - 1);

            vsg::info("encoding = ", encoding);

            // value = buffer.substr(comma+1, end_of_value - comma -1);

            value = memeType;
            object = vsg::stringValue::create(buffer.substr(comma + 1, end_of_value - comma - 1));

            pos = end_of_value + 1;

            vsg::info("value = ", value, ", object = ", object);

            return true;
        }
    }

    value = buffer.substr(pos + 1, end_of_value - pos - 1);

    object = vsg::read(value, options);

    vsg::info("read_uri(", pos, ") -> value = [", value, "] & object = ", object);

    pos = end_of_value + 1;

    return true;
}

bool JSONParser::read_string_view(std::string_view& value)
{
    if (buffer[pos] != '"') return false;

    // read string
    auto end_of_value = buffer.find('"', pos + 1);
    if (end_of_value == std::string::npos) return false;

    value = std::string_view(&buffer[pos + 1], end_of_value - pos - 1);

    pos = end_of_value + 1;

    return true;
}

bool JSONParser::read_string(std::string& value)
{
    if (buffer[pos] != '"') return false;

/*
    https://www.json.org/json-en.html

    Control characters:
    \" -> "
    \\ -> \
    \/ -> /
    \b -> backspace
    \f -> formfeed
    \n -> linefeed
    \r -> carraige return
    \t -> horizontal tab
    \u -> 4 hex digits
*/

    // read string
    auto end_of_value = buffer.find('"', pos + 1);
    if (end_of_value == std::string::npos) return false;

    // TODO: need to add support for escape characters.

    value = buffer.substr(pos + 1, end_of_value - pos - 1);

    pos = end_of_value + 1;

    return true;
}

void JSONParser::read_object(JSONParser::Schema& schema)
{
    if (pos == std::string::npos) return;
    if (buffer[pos] != '{') return;

    // buffer[pos] == '{'
    // advance past open bracket
    pos = buffer.find_first_not_of(" \t\r\n", pos + 1);
    if (pos == std::string::npos) return;

    while (pos != std::string::npos && pos < buffer.size() && buffer[pos] != '}')
    {
        auto previous_position = pos;

        if (buffer[pos] == '"')
        {
            auto end_of_string = buffer.find('"', pos + 1);
            if (end_of_string == std::string::npos) break;

            std::string_view name(&buffer[pos + 1], end_of_string - pos - 1);

            // skip white space
            pos = buffer.find_first_not_of(" \t\r\n", end_of_string + 1);
            if (pos == std::string::npos)
            {
                warning("read_object()  deliminator error end of buffer.");
                break;
            }

            // make sure next charater is the {name : value} deliminator
            if (buffer[pos] != ':')
            {
                warning("read_object()  deliminator error buffer[", pos, "] = ", buffer[pos]);
                break;
            }

            // skip white space
            pos = buffer.find_first_not_of(" \t\r\n", pos + 1);
            if (pos == std::string::npos)
            {
                break;
            }

            // now look to pair with value after " : "
            if (buffer[pos] == '{')
            {
                schema.read_object(*this, name);
            }
            else if (buffer[pos] == '[')
            {
                schema.read_array(*this, name);
            }
            else if (buffer[pos] == '"')
            {
                schema.read_string(*this, name);
            }
            else
            {
                auto end_of_field = buffer.find_first_of(",}]", pos + 1);
                if (end_of_field == std::string::npos) break;

                auto end_of_value = end_of_field - 1;
                while (end_of_value > 0 && white_space(buffer[end_of_value])) --end_of_value;

                if (buffer.compare(pos, end_of_value - pos + 1, "null") == 0)
                {
                    schema.read_null(*this, name);
                }
                else if (buffer.compare(pos, end_of_value - pos + 1, "true") == 0)
                {
                    schema.read_bool(*this, name, true);
                }
                else if (buffer.compare(pos, end_of_value - pos + 1, "false") == 0)
                {
                    schema.read_bool(*this, name, false);
                }
                else
                {
                    mstr.set(reinterpret_cast<const uint8_t*>(&buffer.at(pos)), end_of_value - pos + 1);
                    schema.read_number(*this, name, mstr);
                }

                // skip to end of field
                pos = end_of_field;
            }
        }
        else if (buffer[pos] == ',')
        {
            ++pos;
        }
        else
        {
            warning("read_object() buffer[", pos, "] = ", buffer[pos]);
        }

        pos = buffer.find_first_not_of(" \t\r\n", pos);

        if (pos <= previous_position)
        {
            warning("Parser stuck when reading object.");
            break;
        }
    }

    if (pos < buffer.size() && buffer[pos] == '}')
    {
        ++pos;
    }
}

void JSONParser::read_array(JSONParser::Schema& schema)
{
    pos = buffer.find_first_not_of(" \t\r\n", pos);
    if (pos == std::string::npos) return;
    if (buffer[pos] != '[')
    {
        warning("read_array() could not match opening [");
        return;
    }

    // buffer[pos] == '['
    // advance past open bracket
    pos = buffer.find_first_not_of(" \t\r\n", pos + 1);
    if (pos == std::string::npos)
    {
        warning("read_array() contents after [");
        return;
    }

    while (pos != std::string::npos && pos < buffer.size() && buffer[pos] != ']')
    {
        auto previous_position = pos;

        // now look to pair with value after " : "
        if (buffer[pos] == '{')
        {
            schema.read_object(*this);
        }
        else if (buffer[pos] == '[')
        {
            schema.read_array(*this);
        }
        else if (buffer[pos] == '"')
        {
            schema.read_string(*this);
        }
        else if (buffer[pos] == ',')
        {
            ++pos;
        }
        else
        {
            auto end_of_field = buffer.find_first_of(",}]", pos + 1);
            if (end_of_field == std::string::npos) break;

            auto end_of_value = end_of_field - 1;
            while (end_of_value > 0 && white_space(buffer[end_of_value])) --end_of_value;

            if (buffer.compare(pos, end_of_value - pos + 1, "null") == 0)
            {
                schema.read_null(*this);
            }
            else if (buffer.compare(pos, end_of_value - pos + 1, "true") == 0)
            {
                schema.read_bool(*this, true);
            }
            else if (buffer.compare(pos, end_of_value - pos + 1, "false") == 0)
            {
                schema.read_bool(*this, false);
            }
            else
            {
                mstr.set(reinterpret_cast<const uint8_t*>(&buffer.at(pos)), end_of_value - pos + 1);

                schema.read_number(*this, mstr);
            }

            // skip to end of field
            pos = end_of_field;
        }

        pos = buffer.find_first_not_of(" \t\r\n", pos);

        if (pos <= previous_position)
        {
            warning("Parser stuck when reading array.");
            break;
        }
    }

    if (pos < buffer.size() && buffer[pos] == ']')
    {
        ++pos;
    }
}

std::pair<std::size_t, std::size_t> JSONParser::lineAndColumnAtPosition(std::size_t position) const
{
    std::size_t lineNumber = 1;
    std::size_t startOfLine = 0;
    auto end_of_line = buffer.find_first_of("\n\r", 0);
    while (end_of_line != std::string::npos && (end_of_line < position) && (end_of_line + 1) < buffer.size())
    {
        if (buffer[end_of_line] == '\n') ++end_of_line;
        if (buffer[end_of_line] == '\r') ++end_of_line;
        startOfLine = end_of_line;

        end_of_line = buffer.find_first_of("\n\r", end_of_line);
        if (end_of_line != std::string::npos && end_of_line < buffer.size()) ++lineNumber;
    }
    return {lineNumber, position - startOfLine};
}

std::string_view JSONParser::lineEnclosingPosition(std::size_t position) const
{
    auto start_of_line = buffer.find_last_of("\n\r", position);
    auto end_of_line = buffer.find_first_of("\n\r", position);

    if (start_of_line == std::string::npos)
        start_of_line = 0;
    else
        start_of_line = buffer.find_first_not_of("\n\r", start_of_line);

    if (end_of_line == std::string::npos) end_of_line = buffer.size();

    return std::string_view(&buffer[start_of_line], end_of_line - start_of_line);
}
