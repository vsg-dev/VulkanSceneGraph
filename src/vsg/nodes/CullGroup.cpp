/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/io/stream.h>
#include <vsg/nodes/CullGroup.h>

using namespace vsg;

CullGroup::CullGroup(Allocator* allocator) :
    Inherit(allocator)
{
}

CullGroup::CullGroup(const dsphere& in_bound, Allocator* allocator) :
    Inherit(allocator)
{
    bound = in_bound;
}

CullGroup::~CullGroup()
{
}

void CullGroup::read(Input& input)
{
    Group::read(input);

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("bound", bound);
    }
    else
    {
        input.read("Bound", bound);
    }
}

void CullGroup::write(Output& output) const
{
    Group::write(output);

    if (output.version_greater_equal(0, 1, 4))
    {
        output.write("bound", bound);
    }
    else
    {
        output.write("Bound", bound);
    }
}
