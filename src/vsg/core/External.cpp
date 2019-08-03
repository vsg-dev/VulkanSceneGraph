/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/External.h>

#include <vsg/io/Input.h>
#include <vsg/io/Output.h>

using namespace vsg;

External::External()
{
}

External::External(Allocator* allocator) :
    Inherit(allocator)
{
}

External::External(const std::string& filename, ref_ptr<Object> object):
    _filename(filename),
    _object(object)
{
}

External::~External()
{
}

void External::read(Input& input)
{
    Object::read(input);

    input.read("Filename", _filename);

    if (!_filename.empty())
    {
        _object = input.readFile(_filename);
    }
}

void External::write(Output& output) const
{
    Object::write(output);

    output.write("Filename", _filename);

    // if we should write out object then need to invoke ReaderWriter for it.
    if (!_filename.empty() && _object.valid())
    {
        output.writeFile(_object, _filename);
    }
}
