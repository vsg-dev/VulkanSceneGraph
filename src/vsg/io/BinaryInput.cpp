/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/BinaryInput.h>
#include <vsg/io/Logger.h>
#include <vsg/io/ReaderWriter.h>

#include <cstring>

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

void BinaryInput::_read(std::wstring& value)
{
    std::string string_value;
    _read(string_value);
    convert_utf(string_value, value);
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

void BinaryInput::read(size_t num, std::wstring* value)
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

void BinaryInput::read(size_t num, Path* value)
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


struct double_64
{
    uint8_t sign : 1;
    uint16_t exponent : 11;
    uint64_t mantissa : 52;
};

struct double_128
{
    uint8_t sign : 1;
    uint16_t exponent : 15;
    union
    {
        uint64_t mantissa_64 : 52;
        uint8_t mantissa[14];
    };
};


void BinaryInput::read(size_t num, long double* value)
{
    uint32_t native_type = native_long_double_bits();

    uint32_t read_type;
    _read(1, &read_type);

    if (read_type == native_type)
    {
        info("reading native long double without conversion.");
        _read(num, value);
    }
    else
    {
        if (read_type == 64)
        {
            // 64 to 80
            // 64 to 128
            std::vector<double> data(num);
            _read(num, data.data());

            for(auto& v : data)
            {
                *(value++) = v;
            }
        }
        else // (read_type == 80) || read_type ==128
        {
            std::vector<double_128> data(num);

            _input.read(reinterpret_cast<char*>(data.data()), num * sizeof(double_128));

            if (native_type == 64)
            {
                double_64* dest = reinterpret_cast<double_64*>(value);
                for(auto& v : data)
                {
                    auto& d = *(dest++);
                    d.sign = v.sign;
                    d.exponent = v.exponent;
                    d.mantissa = v.mantissa_64;
                }
            }
            else if (native_type == 80)
            {
                double_128* dest = reinterpret_cast<double_128*>(value);
                for(auto& v : data)
                {
                    auto& d = *(dest++);
                    d.sign = v.sign;
                    d.exponent = v.exponent;

                    // copy fraction and fill in zeros at end
                    for(int i=0; i<8; ++i) d.mantissa[i] = v.mantissa[i];
                }
            }
            else if (native_type == 128)
            {
                double_128* dest = reinterpret_cast<double_128*>(value);
                for(auto& v : data)
                {
                    auto& d = *(dest++);
                    d.sign = v.sign;
                    d.exponent = v.exponent;

                    // copy fraction and fill in zeros at end
                    int i=0;
                    for(; i<8; ++i) d.mantissa[i] = v.mantissa[i];
                    for(; i<14; ++i) d.mantissa[i] = 0;
                }
            }
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
        if (className != "nullptr")
        {
            auto object = objectFactory->create(className.c_str());
            objectIDMap[id] = object;
            if (object)
            {
                object->read(*this);
            }
            else
            {
                warn("Unable to create instance of class : ", className);
            }
            return object;
        }
        else
        {
            return objectIDMap[id] = {};
        }
    }
}
