/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/state/Descriptor.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

Descriptor::Descriptor(uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    dstBinding(in_dstBinding),
    dstArrayElement(in_dstArrayElement),
    descriptorType(in_descriptorType)
{
}

void Descriptor::read(Input& input)
{
    Object::read(input);

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("dstBinding", dstBinding);
        input.read("dstArrayElement", dstArrayElement);
    }
    else
    {
        input.read("DstBinding", dstBinding);
        input.read("DstArrayElement", dstArrayElement);
    }
}

void Descriptor::write(Output& output) const
{
    Object::write(output);

    if (output.version_greater_equal(0, 1, 4))
    {
        output.write("dstBinding", dstBinding);
        output.write("dstArrayElement", dstArrayElement);
    }
    else
    {
        output.write("DstBinding", dstBinding);
        output.write("DstArrayElement", dstArrayElement);
    }
}

void Descriptor::assignTo(Context& /*context*/, VkWriteDescriptorSet& wds) const
{
    wds = {};
    wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds.dstBinding = dstBinding;
    wds.dstArrayElement = dstArrayElement;
    wds.descriptorType = descriptorType;
}
