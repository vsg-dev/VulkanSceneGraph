/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Input.h>
#include <vsg/io/Output.h>
#include <vsg/nodes/QuadGroup.h>

using namespace vsg;

QuadGroup::QuadGroup()
{
}

QuadGroup::QuadGroup(const QuadGroup& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop)
{
    children[0] = copyop(rhs.children[0]);
    children[1] = copyop(rhs.children[1]);
    children[2] = copyop(rhs.children[2]);
    children[3] = copyop(rhs.children[3]);
}

QuadGroup::~QuadGroup()
{
}

int QuadGroup::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_pointer_container(children, rhs.children);
}

void QuadGroup::read(Input& input)
{
    Node::read(input);

    for (auto& child : children)
    {
        input.readObject("Child", child);
    }
}

void QuadGroup::write(Output& output) const
{
    Node::write(output);

    for (const auto& child : children)
    {
        output.writeObject("Child", child.get());
    }
}
