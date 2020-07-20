/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/PipelineBarrier.h>
#include <vsg/state/DescriptorImageView.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/io/Options.h>

using namespace vsg;

DescriptorImageView::DescriptorImageView() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
    _compiled(false)
{
}

DescriptorImageView::DescriptorImageView(ImageData imageData, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType),
    _compiled(false)
{
    _imageDataList.push_back(imageData);
}

void DescriptorImageView::read(Input& input)
{
    _imageDataList.clear();

    Descriptor::read(input);
}

void DescriptorImageView::write(Output& output) const
{
    Descriptor::write(output);
}

void DescriptorImageView::compile(Context& context)
{
    // check if we have already compiled the imageData.
    if (_compiled) return;

    _compiled = true;

    // transfer any data that is required
    for (size_t i = 0; i < _imageDataList.size(); ++i)
    {
        if (_imageDataList[i]._sampler) _imageDataList[i]._sampler->compile(context);

        if (_imageDataList[i]._imageView)
        {
            auto imb_transitionLayoutMemoryBarrier = ImageMemoryBarrier::create(
                0, VK_ACCESS_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, _imageDataList[i]._imageLayout,
                VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                ref_ptr<Image>(_imageDataList[i]._imageView->getImage()),
                VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

            auto pb_transitionLayoutMemoryBarrier = PipelineBarrier::create(
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0, imb_transitionLayoutMemoryBarrier);

            context.commands.emplace_back(pb_transitionLayoutMemoryBarrier);
        }
    }
}

void DescriptorImageView::assignTo(Context& context, VkWriteDescriptorSet& wds) const
{
    Descriptor::assignTo(context, wds);

    // convert from VSG to Vk
    auto pImageInfo = context.scratchMemory->allocate<VkDescriptorImageInfo>(_imageDataList.size());
    wds.descriptorCount = static_cast<uint32_t>(_imageDataList.size());
    wds.pImageInfo = pImageInfo;

    for (size_t i = 0; i < _imageDataList.size(); ++i)
    {
        const ImageData& data = _imageDataList[i];
        VkDescriptorImageInfo& info = pImageInfo[i];
        if (data._sampler)
            info.sampler = data._sampler->vk(context.deviceID);
        else
            info.sampler = 0;

        if (data._imageView)
            info.imageView = *(data._imageView);
        else
            info.imageView = 0;

        info.imageLayout = data._imageLayout;
    }
}

uint32_t DescriptorImageView::getNumDescriptors() const
{
    return static_cast<uint32_t>(_imageDataList.size());
}
