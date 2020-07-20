/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/DescriptorImage.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/io/Options.h>

using namespace vsg;

DescriptorImage::DescriptorImage() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
{
}

DescriptorImage::DescriptorImage(ref_ptr<Sampler> sampler, ref_ptr<Data> image, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType)
{
    if (sampler || image) _samplerImages.emplace_back(SamplerImage{sampler, image});
}

DescriptorImage::DescriptorImage(const SamplerImage& samplerImage, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType)
{
    if (samplerImage.sampler || samplerImage.data) _samplerImages.emplace_back(samplerImage);
}

DescriptorImage::DescriptorImage(const SamplerImages& samplerImages, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType),
    _samplerImages(samplerImages)
{
}

void DescriptorImage::read(Input& input)
{
    _vulkanData.clear();

    Descriptor::read(input);

    _samplerImages.resize(input.readValue<uint32_t>("NumImages"));
    for (auto& samplerImage : _samplerImages)
    {
        input.readObject("Sampler", samplerImage.sampler);
        input.readObject("Image", samplerImage.data);
    }
}

void DescriptorImage::write(Output& output) const
{
    Descriptor::write(output);

    output.writeValue<uint32_t>("NumImages", _samplerImages.size());
    for (auto& samplerImage : _samplerImages)
    {
        output.writeObject("Sampler", samplerImage.sampler.get());
        output.writeObject("Image", samplerImage.data.get());
    }
}

void DescriptorImage::compile(Context& context)
{
    if (_samplerImages.empty()) return;

    auto& vkd = _vulkanData[context.deviceID];

    // check if we have already compiled the imageData.
    if (vkd.imageDataList.size() == _samplerImages.size()) return;

    if (!_samplerImages.empty())
    {
        vkd.imageDataList.clear();
        vkd.imageDataList.reserve(_samplerImages.size());
        for (auto& samplerImage : _samplerImages)
        {
            samplerImage.sampler->compile(context);
            vkd.imageDataList.emplace_back(vsg::transferImageData(context, samplerImage.data, samplerImage.sampler));
        }
    }
}

void DescriptorImage::assignTo(Context& context, VkWriteDescriptorSet& wds) const
{
    Descriptor::assignTo(context, wds);

    auto& vkd = _vulkanData[context.deviceID];

    // convert from VSG to Vk
    auto pImageInfo = context.scratchMemory->allocate<VkDescriptorImageInfo>(vkd.imageDataList.size());
    wds.descriptorCount = static_cast<uint32_t>(vkd.imageDataList.size());
    wds.pImageInfo = pImageInfo;
    for (size_t i = 0; i < vkd.imageDataList.size(); ++i)
    {
        const ImageData& data = vkd.imageDataList[i];

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

uint32_t DescriptorImage::getNumDescriptors() const
{
    return static_cast<uint32_t>(_samplerImages.size());
}
