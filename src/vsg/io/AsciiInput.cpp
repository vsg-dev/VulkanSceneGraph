/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/AsciiInput.h>
#include <vsg/io/Logger.h>
#include <vsg/io/ReaderWriter.h>

#include <cstring>

using namespace vsg;

AsciiInput::AsciiInput(std::istream& input, ref_ptr<ObjectFactory> in_objectFactory, ref_ptr<const Options> in_options) :
    Input(in_objectFactory, in_options),
    _input(input)
{
}

bool AsciiInput::matchPropertyName(const char* propertyName)
{
    _input >> _readPropertyName;
    if (_readPropertyName != propertyName)
    {
        error("Unable to match ", propertyName, " got ", _readPropertyName, " instead.");
        return false;
    }
    return true;
}

AsciiInput::OptionalObjectID AsciiInput::objectID()
{
    std::string token;
    _input >> token;
    if (token.compare(0, 3, "id=") == 0)
    {
        token.erase(0, 3);
        std::stringstream str(token);
        ObjectID id;
        str >> id;
        return OptionalObjectID{true, id};
    }
    else
    {
        return OptionalObjectID(false, 0);
    }
}

void AsciiInput::_read(std::string& value)
{
    value.clear();

    char c;
    _input >> c;
    if (_input.good())
    {
        if (c == '"')
        {
            _input.get(c);
            while (_input.good())
            {
                if (c == '\\')
                {
                    _input.get(c);
                    if (c == '"')
                        value.push_back(c);
                    else
                    {
                        value.push_back('\\');
                        value.push_back(c);
                    }
                }
                else if (c != '"')
                {
                    value.push_back(c);
                }
                else
                {
                    break;
                }
                _input.get(c);
            }
        }
        else
        {
            _input >> value;
        }
    }
}

void AsciiInput::read(size_t num, std::string* value)
{
    if (num == 1)
    {
        _read(*value);
    }
    else
    {
        for (; num > 0; --num, ++value)
        {
            _read(*value);
        }
    }
}

void AsciiInput::_read(std::wstring& value)
{
    std::string string_value;
    _read(string_value);
    convert_utf(string_value, value);
}

void AsciiInput::read(size_t num, std::wstring* value)
{
    if (num == 1)
    {
        _read(*value);
    }
    else
    {
        for (; num > 0; --num, ++value)
        {
            _read(*value);
        }
    }
}

void AsciiInput::read(size_t num, Path* value)
{
    if (num == 1)
    {
        std::string str_value;
        _read(str_value);
        *value = str_value;
    }
    else
    {
        for (; num > 0; --num, ++value)
        {
            std::string str_value;
            _read(str_value);
            *value = str_value;
        }
    }
}

vsg::ref_ptr<vsg::Object> AsciiInput::read()
{
    auto result = objectID();
    if (result.first)
    {
        ObjectID id = result.second;
        //debug("   matched result=", id);

        if (auto itr = objectIDMap.find(id); itr != objectIDMap.end())
        {
            //debug("Returning existing object ", itr->second);
            return itr->second;
        }
        else
        {
            std::string className;
            _input >> className;

            //debug("Loading new object ", className);

            vsg::ref_ptr<vsg::Object> object;

            if (className != "nullptr")
            {
                object = objectFactory->create(className.c_str());

                if (object)
                {
                    matchPropertyName("{");

                    object->read(*this);

                    //debug("Loaded object, assigning to objectIDMap.", object);

                    matchPropertyName("}");
                }
                else
                {
                    warn("Could not find means to create ", className);
                }
            }

            objectIDMap[id] = object;

            return object;
        }
    }
    return vsg::ref_ptr<vsg::Object>();
}
