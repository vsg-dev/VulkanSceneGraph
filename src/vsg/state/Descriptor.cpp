/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/state/Descriptor.h>
#include <vsg/vk/Context.h>

using namespace vsg;

Descriptor::Descriptor(uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    dstBinding(in_dstBinding),
    dstArrayElement(in_dstArrayElement),
    descriptorType(in_descriptorType)
{
}

Descriptor::Descriptor(const Descriptor& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    dstBinding(rhs.dstBinding),
    dstArrayElement(rhs.dstArrayElement),
    descriptorType(rhs.descriptorType)
{
}

int Descriptor::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(dstBinding, rhs.dstBinding))) return result;
    if ((result = compare_value(dstArrayElement, rhs.dstArrayElement))) return result;
    return compare_value(descriptorType, rhs.descriptorType);
}

void Descriptor::read(Input& input)
{
    Object::read(input);

    input.read("dstBinding", dstBinding);
    input.read("dstArrayElement", dstArrayElement);
}

void Descriptor::write(Output& output) const
{
    Object::write(output);

    output.write("dstBinding", dstBinding);
    output.write("dstArrayElement", dstArrayElement);
}

void Descriptor::assignTo(Context& /*context*/, VkWriteDescriptorSet& wds) const
{
    wds = {};
    wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds.dstBinding = dstBinding;
    wds.dstArrayElement = dstArrayElement;
    wds.descriptorType = descriptorType;
}
