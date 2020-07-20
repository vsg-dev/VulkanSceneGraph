/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/Descriptor.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/io/Options.h>

using namespace vsg;

Descriptor::Descriptor(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    _dstBinding(dstBinding),
    _dstArrayElement(dstArrayElement),
    _descriptorType(descriptorType)
{
}

void Descriptor::read(Input& input)
{
    Object::read(input);

    input.read("DstBinding", _dstBinding);
    input.read("DstArrayElement", _dstArrayElement);
}

void Descriptor::write(Output& output) const
{
    Object::write(output);

    output.write("DstBinding", _dstBinding);
    output.write("DstArrayElement", _dstArrayElement);
}

void Descriptor::assignTo(Context& /*context*/, VkWriteDescriptorSet& wds) const
{
    wds = {};
    wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds.dstBinding = _dstBinding;
    wds.dstArrayElement = _dstArrayElement;
    wds.descriptorType = _descriptorType;
}
