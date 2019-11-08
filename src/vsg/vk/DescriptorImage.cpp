/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/DescriptorImage.h>

#include <algorithm>
#include <iostream>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorImages
//
DescriptorImage::DescriptorImage() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
{
}

DescriptorImage::DescriptorImage(ref_ptr<Sampler> sampler, ref_ptr<Data> image, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType)
{
    if (sampler || image) _samplerImages.emplace_back(SamplerImage {sampler, image, ref_ptr<ImageView>()});
}

DescriptorImage::DescriptorImage(ref_ptr<Sampler> sampler, ref_ptr<ImageView> image, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType)
{
    if (sampler || image) _samplerImages.emplace_back(SamplerImage{sampler, ref_ptr<Data>(), image});
}

DescriptorImage::DescriptorImage(const SamplerImage& samplerImage, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType)
{
    if (samplerImage.sampler || samplerImage.data || samplerImage.imageView) _samplerImages.emplace_back(samplerImage);
}

DescriptorImage::DescriptorImage(const SamplerImages& samplerImages, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType),
    _samplerImages(samplerImages)
{
}

void DescriptorImage::read(Input& input)
{
    _imageDataList.clear();
    _imageInfos.clear();

    Descriptor::read(input);

    _samplerImages.resize(input.readValue<uint32_t>("NumImages"));
    for (auto& samplerImage : _samplerImages)
    {
        samplerImage.sampler = input.readObject<Sampler>("Sampler");
        samplerImage.data = input.readObject<Data>("Image");
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
    // check if we have already compiled the imageData.
    if ((_imageInfos.size() >= _imageDataList.size()) && (_imageInfos.size() >= _samplerImages.size())) return;

    if (!_samplerImages.empty())
    {
        _imageDataList.clear();
        _imageDataList.reserve(_samplerImages.size());
        for (auto& samplerImage : _samplerImages)
        {
            samplerImage.sampler->compile(context);
            if (samplerImage.imageView)
            {
                ImageData imagedata = ImageData(samplerImage.sampler, samplerImage.imageView);
                imagedata._imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //no way to set this from an image view at the moment
                _imageDataList.emplace_back(imagedata);
                context.commands.emplace_back(new SetImageLayoutCommand(imagedata));
            }
            else
            {
                _imageDataList.emplace_back(vsg::transferImageData(context, samplerImage.data, samplerImage.sampler));
            }
        }
    }

    // convert from VSG to Vk
    _imageInfos.resize(_imageDataList.size());
    for (size_t i = 0; i < _imageDataList.size(); ++i)
    {
        const ImageData& data = _imageDataList[i];
        VkDescriptorImageInfo& info = _imageInfos[i];
        if (data._sampler)
            info.sampler = *(data._sampler);
        else
            info.sampler = 0;

        if (data._imageView)
            info.imageView = *(data._imageView);
        else
            info.imageView = 0;

        info.imageLayout = data._imageLayout;
    }
}

bool DescriptorImage::assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
{
    Descriptor::assignTo(wds, descriptorSet);
    wds.descriptorCount = static_cast<uint32_t>(_imageInfos.size());
    wds.pImageInfo = _imageInfos.data();
    return true;
}

uint32_t DescriptorImage::getNumDescriptors() const
{
    return static_cast<uint32_t>(std::max(_imageDataList.size(), _samplerImages.size()));
}
