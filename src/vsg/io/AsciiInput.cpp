/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/AsciiInput.h>

#include <cstring>
#include <iostream>
#include <sstream>

using namespace vsg;

AsciiInput::AsciiInput(std::istream& input) :
    _input(input)
{
    _input.imbue(std::locale::classic());

    _objectFactory = new vsg::ObjectFactory;

    // write header
    const char* match_token = "#vsga";
    char read_token[5];
    _input.read(read_token, 5);
    if (std::strncmp(match_token, read_token, 5) != 0)
    {
        std::cout << "Header token not matched" << std::endl;
        throw std::string("Error: header not matched.");
    }

    char read_line[1024];
    _input.getline(read_line, sizeof(read_line) - 1);
    std::cout << "First line [" << read_line << "]" << std::endl;
}

bool AsciiInput::matchPropertyName(const char* propertyName)
{
    _input >> _readPropertyName;
    if (_readPropertyName != propertyName)
    {
        std::cout << "Error: unable to match " << propertyName << std::endl;
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

vsg::ref_ptr<vsg::Object> AsciiInput::read()
{
    auto result = objectID();
    if (result.first)
    {
        ObjectID id = result.second;
        //std::cout<<"   matched result="<<id<<std::endl;

        if (auto itr = _objectIDMap.find(id); itr != _objectIDMap.end())
        {
            //std::cout<<"Returning existing object "<<itr->second.get()<<std::endl;
            return itr->second;
        }
        else
        {
            std::string className;
            _input >> className;

            //std::cout<<"Loading new object "<<className<<std::endl;

            vsg::ref_ptr<vsg::Object> object;

            if (className != "nullptr")
            {
                object = _objectFactory->create(className.c_str());

                if (object)
                {
                    matchPropertyName("{");

                    object->read(*this);

                    //std::cout<<"Loaded object, assigning to _objectIDMap."<<object.get()<<std::endl;

                    matchPropertyName("}");
                }
                else
                {
                    std::cout << "Could not find means to create " << className.c_str() << std::endl;
                }
            }

            _objectIDMap[id] = object;

            return object;
        }
    }
    return vsg::ref_ptr<vsg::Object>();
}
