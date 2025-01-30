/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/state/DescriptorTexelBufferView.h>
#include <vsg/vk/Context.h>

using namespace vsg;

DescriptorTexelBufferView::DescriptorTexelBufferView() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER)
{
}

DescriptorTexelBufferView::DescriptorTexelBufferView(const DescriptorTexelBufferView& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    texelBufferViews(copyop(rhs.texelBufferViews))
{
}

DescriptorTexelBufferView::DescriptorTexelBufferView(uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType, const BufferViewList& in_texelBufferViews) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType),
    texelBufferViews(in_texelBufferViews)
{
}

int DescriptorTexelBufferView::compare(const Object& rhs_object) const
{
    int result = Descriptor::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_pointer_container(texelBufferViews, rhs.texelBufferViews);
}
void DescriptorTexelBufferView::read(Input& input)
{
    Descriptor::read(input);

    input.readObjects("texelBufferViews", texelBufferViews);
}

void DescriptorTexelBufferView::write(Output& output) const
{
    Descriptor::write(output);

    output.writeObjects("texelBufferViews", texelBufferViews);
}

void DescriptorTexelBufferView::compile(Context& context)
{
    if (texelBufferViews.empty()) return;

    for (auto& bufferView : texelBufferViews)
    {
        bufferView->compile(context);
    }
}

void DescriptorTexelBufferView::assignTo(Context& context, VkWriteDescriptorSet& wds) const
{
    Descriptor::assignTo(context, wds);

    auto vk_texelBufferViews = context.scratchMemory->allocate<VkBufferView>(texelBufferViews.size());
    wds.descriptorCount = static_cast<uint32_t>(texelBufferViews.size());
    wds.pTexelBufferView = vk_texelBufferViews;

    for (size_t i = 0; i < texelBufferViews.size(); ++i)
    {
        texelBufferViews[i]->compile(context);
        vk_texelBufferViews[i] = texelBufferViews[i]->vk(context.deviceID);
    }
}
