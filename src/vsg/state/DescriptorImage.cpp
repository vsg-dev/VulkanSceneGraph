/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/CopyAndReleaseImage.h>
#include <vsg/io/Options.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorImage
//
DescriptorImage::DescriptorImage() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
{
}

DescriptorImage::DescriptorImage(ref_ptr<Sampler> sampler, ref_ptr<Data> data, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType)
{
    if (sampler && data)
    {
        auto image = Image::create(data);
        image->usage |= (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

        auto imageView = ImageView::create(image);
        imageInfoList.emplace_back(ImageInfo{sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
    }
}

DescriptorImage::DescriptorImage(const ImageInfo& imageInfo, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType)
{
    imageInfoList.push_back(imageInfo);
}

DescriptorImage::DescriptorImage(const ImageInfoList& in_imageInfoList, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType),
    imageInfoList(in_imageInfoList)
{
}

DescriptorImage::DescriptorImage(const SamplerImage& si, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType)
{
    if (si.sampler && si.data)
    {
        auto image = Image::create(si.data);
        image->usage |= (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

        auto imageView = ImageView::create(image);
        imageInfoList.emplace_back(ImageInfo{si.sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
    }
}

DescriptorImage::DescriptorImage(const SamplerImages& samplerImages, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType)
{
    for (auto& si : samplerImages)
    {
        if (si.sampler && si.data)
        {
            auto image = Image::create(si.data);
            image->usage |= (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

            auto imageView = ImageView::create(image);
            imageInfoList.emplace_back(ImageInfo{si.sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
        }
    }
}

void DescriptorImage::read(Input& input)
{
    // TODO need to release on imageInfoList.

    Descriptor::read(input);

    // TODO old version

    imageInfoList.resize(input.readValue<uint32_t>("NumImages"));
    for (auto& imageData : imageInfoList)
    {
        ref_ptr<Data> data;
        input.readObject("Sampler", imageData.sampler);
        input.readObject("Image", data);

        auto image = Image::create(data);
        if (imageData.sampler) image->usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        image->usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        imageData.imageView = ImageView::create(image);
        imageData.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
}

void DescriptorImage::write(Output& output) const
{
    Descriptor::write(output);

    // TODO old version

    output.writeValue<uint32_t>("NumImages", imageInfoList.size());
    for (auto& imageData : imageInfoList)
    {
        output.writeObject("Sampler", imageData.sampler.get());

        ref_ptr<Data> data;
        if (imageData.imageView && imageData.imageView->image) data = imageData.imageView->image->data;

        output.writeObject("Image", data.get());
    }
}

void DescriptorImage::compile(Context& context)
{
    if (imageInfoList.empty()) return;

    for (auto& imageData : imageInfoList)
    {
        if (imageData.sampler) imageData.sampler->compile(context);
        if (imageData.imageView)
        {
            auto imageView = imageData.imageView;
            if (imageView->image && imageView->image->data)
            {
                imageData.computeNumMipMapLevels();

                auto image = imageView->image;

                imageView->compile(context);

                if (image && image->data)
                {
                    auto stagingBufferInfo = copyDataToStagingBuffer(context, image->data);
                    if (stagingBufferInfo)
                    {
                        context.commands.emplace_back(CopyAndReleaseImage::create(stagingBufferInfo, imageData, image->mipLevels));
                    }
                }
            }
            else
            {
                imageView->compile(context);
            }
        }
    }
}

void DescriptorImage::assignTo(Context& context, VkWriteDescriptorSet& wds) const
{
    Descriptor::assignTo(context, wds);

    // convert from VSG to Vk
    auto pImageInfo = context.scratchMemory->allocate<VkDescriptorImageInfo>(imageInfoList.size());
    wds.descriptorCount = static_cast<uint32_t>(imageInfoList.size());
    wds.pImageInfo = pImageInfo;
    for (size_t i = 0; i < imageInfoList.size(); ++i)
    {
        const ImageInfo& data = imageInfoList[i];

        VkDescriptorImageInfo& info = pImageInfo[i];
        if (data.sampler)
            info.sampler = data.sampler->vk(context.deviceID);
        else
            info.sampler = 0;

        if (data.imageView)
            info.imageView = data.imageView->vk(context.deviceID);
        else
            info.imageView = 0;

        info.imageLayout = data.imageLayout;
    }
}

uint32_t DescriptorImage::getNumDescriptors() const
{
    return static_cast<uint32_t>(imageInfoList.size());
}
