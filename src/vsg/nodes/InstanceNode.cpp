/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/stream.h>
#include <vsg/nodes/InstanceNode.h>

using namespace vsg;

InstanceNode::InstanceNode()
{
}

InstanceNode::InstanceNode(const InstanceNode& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    bound(rhs.bound),
    translations(copyop(rhs.translations)),
    rotations(copyop(rhs.rotations)),
    scales(copyop(rhs.scales)),
    colors(copyop(rhs.colors)),
    child(copyop(rhs.child))
{
}

InstanceNode::InstanceNode(const dsphere& in_bound, Node* in_child) :
    bound(in_bound),
    child(in_child)
{
}

InstanceNode::~InstanceNode()
{
}

int InstanceNode::compare(const Object& rhs_object) const
{
    int result = Node::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(bound, rhs.bound)) != 0) return result;
    if ((result = compare_pointer(translations, rhs.translations)) != 0) return result;
    if ((result = compare_pointer(rotations, rhs.rotations)) != 0) return result;
    if ((result = compare_pointer(scales, rhs.scales)) != 0) return result;
    if ((result = compare_pointer(colors, rhs.colors)) != 0) return result;
    return compare_pointer(child, rhs.child);
}

void InstanceNode::read(Input& input)
{
    Node::read(input);

    input.read("bound", bound);

    input.read("translations", translations);
    input.read("rotations", rotations);
    input.read("scales", scales);
    input.read("colors", colors);

    input.read("child", child);
}

void InstanceNode::write(Output& output) const
{
    Node::write(output);

    output.write("bound", bound);

    output.write("translations", translations);
    output.write("rotations", rotations);
    output.write("scales", scales);
    output.write("colors", colors);

    output.write("child", child);
}
