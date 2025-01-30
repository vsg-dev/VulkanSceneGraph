/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/stream.h>
#include <vsg/nodes/Layer.h>

using namespace vsg;

Layer::Layer()
{
}

Layer::Layer(const Layer& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    mask(rhs.mask),
    binNumber(rhs.binNumber),
    value(rhs.value),
    child(copyop(rhs.child))
{
}

Layer::Layer(int32_t in_binNumber, double in_value, ref_ptr<Node> in_child) :
    binNumber(in_binNumber),
    value(in_value),
    child(in_child)
{
}

Layer::~Layer()
{
}

int Layer::compare(const Object& rhs_object) const
{
    int result = Node::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(mask, rhs.mask)) != 0) return result;
    if ((result = compare_value(binNumber, rhs.binNumber)) != 0) return result;
    if ((result = compare_value(value, rhs.value)) != 0) return result;
    return compare_pointer(child, rhs.child);
}

void Layer::read(Input& input)
{
    Node::read(input);

    input.read("mask", mask);
    input.read("binNumber", binNumber);
    input.read("value", value);
    input.readObject("child", child);
}

void Layer::write(Output& output) const
{
    Node::write(output);

    output.write("mask", mask);
    output.write("binNumber", binNumber);
    output.write("value", value);
    output.writeObject("child", child.get());
}
