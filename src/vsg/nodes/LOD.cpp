
/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/stream.h>
#include <vsg/nodes/LOD.h>

using namespace vsg;

LOD::LOD()
{
}

LOD::LOD(const LOD& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    bound(rhs.bound)
{
    children.reserve(rhs.children.size());
    for (auto child : rhs.children)
    {
        children.push_back(Child{child.minimumScreenHeightRatio, copyop(child.node)});
    }
}

LOD::~LOD()
{
}

int LOD::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(bound, rhs.bound)) != 0) return result;

    // compare the children vector
    if (children.size() < rhs.children.size()) return -1;
    if (children.size() > rhs.children.size()) return 1;
    if (children.empty()) return 0;

    auto rhs_itr = rhs.children.begin();
    for (auto lhs_itr = children.begin(); lhs_itr != children.end(); ++lhs_itr, ++rhs_itr)
    {
        if ((result = compare_value(lhs_itr->minimumScreenHeightRatio, rhs_itr->minimumScreenHeightRatio)) != 0) return result;
        if ((result = compare_pointer(lhs_itr->node, rhs_itr->node)) != 0) return result;
    }
    return 0;
}

void LOD::read(Input& input)
{
    Node::read(input);

    input.read("bound", bound);

    children.resize(input.readValue<uint32_t>("children"));
    for (auto& child : children)
    {
        input.read("child.minimumScreenHeightRatio", child.minimumScreenHeightRatio);
        input.read("child.node", child.node);
    }
}

void LOD::write(Output& output) const
{
    Node::write(output);

    output.write("bound", bound);

    output.writeValue<uint32_t>("children", children.size());
    for (auto& child : children)
    {
        output.write("child.minimumScreenHeightRatio", child.minimumScreenHeightRatio);
        output.write("child.node", child.node);
    }
}
