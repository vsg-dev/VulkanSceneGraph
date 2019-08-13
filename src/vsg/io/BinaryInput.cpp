/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/BinaryInput.h>
#include <vsg/io/ReaderWriter.h>

#include <cstring>
#include <iostream>
#include <sstream>

using namespace vsg;

BinaryInput::BinaryInput(std::istream& input, ref_ptr<ObjectFactory> in_objectFactory, ref_ptr<const Options> in_options) :
    Input(in_objectFactory, in_options),
    _input(input)
{
}

void BinaryInput::_read(std::string& value)
{
    uint32_t size = readValue<uint32_t>(nullptr);

    value.resize(size, 0);
    _input.read(value.data(), size);
}

void BinaryInput::read(size_t num, std::string* value)
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

vsg::ref_ptr<vsg::Object> BinaryInput::read()
{
    ObjectID id = objectID();

    if (auto itr = objectIDMap.find(id); itr != objectIDMap.end())
    {
        return itr->second;
    }
    else
    {
        std::string className = readValue<std::string>(nullptr);

        vsg::ref_ptr<vsg::Object> object;
        if (className != "nullptr")
        {
            object = objectFactory->create(className.c_str());
            if (object)
            {
                object->read(*this);
            }
            else
            {
                std::cout << "Unable to create instance of class : " << className << std::endl;
            }
        }

        objectIDMap[id] = object;
        return object;
    }
}
