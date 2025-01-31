/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/CopyAndReleaseImage.h>
#include <vsg/core/compare.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/vk/Context.h>

using namespace vsg;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorImage
//
DescriptorImage::DescriptorImage() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
{
}

DescriptorImage::DescriptorImage(const DescriptorImage& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    imageInfoList(copyop(rhs.imageInfoList))
{
}

DescriptorImage::DescriptorImage(ref_ptr<Sampler> sampler, ref_ptr<Data> data, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType)
{
    if (sampler && data)
    {
        imageInfoList.push_back(ImageInfo::create(sampler, data));
    }
}

DescriptorImage::DescriptorImage(ref_ptr<ImageInfo> imageInfo, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType)
{
    imageInfoList.push_back(imageInfo);
}

DescriptorImage::DescriptorImage(const ImageInfoList& in_imageInfoList, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType) :
    Inherit(in_dstBinding, in_dstArrayElement, in_descriptorType),
    imageInfoList(in_imageInfoList)
{
}

int DescriptorImage::compare(const Object& rhs_object) const
{
    int result = Descriptor::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    return compare_pointer_container(imageInfoList, rhs.imageInfoList);
}

void DescriptorImage::read(Input& input)
{
    imageInfoList.clear();

    Descriptor::read(input);

    imageInfoList.resize(input.readValue<uint32_t>("images"));
    for (auto& imageInfo : imageInfoList)
    {
        imageInfo = ImageInfo::create();

        ref_ptr<Data> data;
        input.readObject("sampler", imageInfo->sampler);
        input.readObject("image", data);

        auto image = Image::create(data);
        if (imageInfo->sampler) image->usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        image->usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        imageInfo->imageView = ImageView::create(image);
        imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
}

void DescriptorImage::write(Output& output) const
{
    Descriptor::write(output);

    output.writeValue<uint32_t>("images", imageInfoList.size());
    for (const auto& imageInfo : imageInfoList)
    {
        output.writeObject("sampler", imageInfo->sampler.get());

        ref_ptr<Data> data;
        if (imageInfo->imageView && imageInfo->imageView->image) data = imageInfo->imageView->image->data;

        output.writeObject("image", data.get());
    }
}

void DescriptorImage::compile(Context& context)
{
    if (imageInfoList.empty()) return;

    auto transferTask = context.transferTask.get();

    for (auto& imageInfo : imageInfoList)
    {
        if (imageInfo->sampler) imageInfo->sampler->compile(context);
        if (imageInfo->imageView)
        {
            if (imageInfo->imageView->image->mipLevels == 0)
            {
                imageInfo->computeNumMipMapLevels();
            }

            auto& imageView = *imageInfo->imageView;
            imageView.compile(context);

            if (!transferTask && imageView.image && imageView.image->syncModifiedCount(context.deviceID))
            {
                const auto& image = *imageView.image;
                context.copy(image.data, imageInfo, image.mipLevels);
            }
        }
    }

    if (transferTask) transferTask->assign(imageInfoList);
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
        auto& imageInfo = imageInfoList[i];

        VkDescriptorImageInfo& info = pImageInfo[i];
        if (imageInfo->sampler)
            info.sampler = imageInfo->sampler->vk(context.deviceID);
        else
            info.sampler = 0;

        if (imageInfo->imageView)
            info.imageView = imageInfo->imageView->vk(context.deviceID);
        else
            info.imageView = 0;

        info.imageLayout = imageInfo->imageLayout;
    }
}

uint32_t DescriptorImage::getNumDescriptors() const
{
    return static_cast<uint32_t>(imageInfoList.size());
}

VSG_DECLSPEC ref_ptr<DescriptorImage> vsg::createSamplerDescriptor(ref_ptr<Sampler> sampler, uint32_t dstBinding, uint32_t dstArrayElement)
{
    ref_ptr<ImageInfo> imageImageInfo = ImageInfo::create(sampler, nullptr, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    return DescriptorImage::create(imageImageInfo, dstBinding, dstArrayElement, VK_DESCRIPTOR_TYPE_SAMPLER);
}

VSG_DECLSPEC ref_ptr<DescriptorImage> vsg::createCombinedImageSamplerDescriptor(ref_ptr<Sampler> sampler, ref_ptr<Data> image, uint32_t dstBinding, uint32_t dstArrayElement)
{
    ref_ptr<ImageInfo> imageImageInfo = ImageInfo::create(sampler, image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    return DescriptorImage::create(imageImageInfo, dstBinding, dstArrayElement, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

VSG_DECLSPEC ref_ptr<DescriptorImage> vsg::createSampedImageDescriptor(ref_ptr<Data> image, uint32_t dstBinding, uint32_t dstArrayElement)
{
    ref_ptr<ImageInfo> imageImageInfo = ImageInfo::create(nullptr, image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    return DescriptorImage::create(imageImageInfo, dstBinding, dstArrayElement, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
}
