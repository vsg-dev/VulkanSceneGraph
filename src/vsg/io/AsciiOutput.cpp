/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Version.h>

#include <vsg/io/AsciiOutput.h>

#include <cstring>

using namespace vsg;

AsciiOutput::AsciiOutput(std::ostream& output, ref_ptr<const Options> in_options) :
    Output(in_options),
    _output(output)
{
    _maximumIndentation = std::strlen(_indentationString);
}

void AsciiOutput::writePropertyName(const char* propertyName)
{
    indent() << propertyName;
}

void AsciiOutput::write(size_t num, const std::string* value)
{
    if (num == 1)
    {
        _output << ' ';
        _write(*value);
    }
    else
    {
        for (; num > 0; --num, ++value)
        {
            _output << ' ';
            _write(*value);
        }
    }
}

void AsciiOutput::write(size_t num, const Path* value)
{
    if (num == 1)
    {
        _output << ' ';
        _write(value->string());
    }
    else
    {
        for (; num > 0; --num, ++value)
        {
            _output << ' ';
            _write(value->string());
        }
    }
}

void AsciiOutput::write(const vsg::Object* object)
{
    if (auto itr = objectIDMap.find(object); itr != objectIDMap.end())
    {
        // write out the objectID
        _output << " id=" << itr->second << "\n";
        return;
    }

    ObjectID id = objectID++;
    objectIDMap[object] = id;

    if (object)
    {
#if 0
        _output<<"id="<<id<<" "<<vsg::type_name(*object)<<"\n";
#else
        _output << " id=" << id << " " << object->className() << "\n";
#endif
        indent() << "{\n";
        _indentation += _indentationStep;
        object->write(*this);
        _indentation -= _indentationStep;
        indent() << "}\n";
    }
    else
    {
        _output << " id=" << id << " nullptr\n";
    }
}
