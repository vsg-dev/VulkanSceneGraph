/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Descriptor.h>

#include <algorithm>
#include <iostream>

using namespace vsg;


/////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorImages
//
DescriptorImage::DescriptorImage():
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
    _imageInfo{0, 0, VK_IMAGE_LAYOUT_UNDEFINED}
{
}

DescriptorImage::DescriptorImage(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, ref_ptr<Sampler> sampler, ref_ptr<Data> image) :
    Inherit(dstBinding, dstArrayElement, descriptorType),
    _samplerImage(sampler, image),
    _imageInfo{0, 0, VK_IMAGE_LAYOUT_UNDEFINED}
{
}

DescriptorImage::DescriptorImage(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const SamplerImage& samplerImage) :
    Inherit(dstBinding, dstArrayElement, descriptorType),
    _samplerImage(samplerImage),
    _imageInfo{0, 0, VK_IMAGE_LAYOUT_UNDEFINED}
{
}

void DescriptorImage::read(Input& input)
{
    _imageData._sampler = nullptr;
    _imageData._imageView = nullptr;

    Descriptor::read(input);

    _samplerImage.first = input.readObject<Sampler>("Sampler");
    _samplerImage.second = input.readObject<Data>("Image");
}

void DescriptorImage::write(Output& output) const
{
    Descriptor::write(output);

    output.writeObject("Sampler", _samplerImage.first.get());
    output.writeObject("Image", _samplerImage.second.get());
}

void DescriptorImage::compile(Context& context)
{
    // check if we have already compiled the imageData.
    if (_imageInfo.sampler || _imageInfo.imageView) return;

    // compile and copy over any sampler + buffer data that is required.
    if (_samplerImage.first || _samplerImage.second)
    {
        if (_samplerImage.first) _samplerImage.first->compile(context);
        _imageData = vsg::transferImageData(context, _samplerImage.second, _samplerImage.first);
    }

    // convert from VSG to Vk
    if (_imageData._sampler) _imageInfo.sampler = *(_imageData._sampler);
    else _imageInfo.sampler = 0;

    if (_imageData._imageView) _imageInfo.imageView = *(_imageData._imageView);
    else _imageInfo.imageView = 0;

    _imageInfo.imageLayout = _imageData._imageLayout;
}

bool DescriptorImage::assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
{
    Descriptor::assignTo(wds, descriptorSet);
    wds.descriptorCount = 1;
    wds.pImageInfo = &_imageInfo;
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorImages
//
DescriptorImages::DescriptorImages():
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
{
}

DescriptorImages::DescriptorImages(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const SamplerImages& samplerImages) :
    Inherit(dstBinding, dstArrayElement, descriptorType),
    _samplerImages(samplerImages)
{
}

void DescriptorImages::read(Input& input)
{
    _imageDataList.clear();
    _imageInfos.clear();

    Descriptor::read(input);

    _samplerImages.resize(input.readValue<uint32_t>("NumImages"));
    for (auto& samplerImage : _samplerImages)
    {
        samplerImage.first = input.readObject<Sampler>("Sampler");
        samplerImage.second = input.readObject<Data>("Image");
    }
}

void DescriptorImages::write(Output& output) const
{
    Descriptor::write(output);

    output.writeValue<uint32_t>("NumImages", _samplerImages.size());
    for (auto& samplerImage : _samplerImages)
    {
        output.writeObject("Sampler", samplerImage.first.get());
        output.writeObject("Image", samplerImage.second.get());
    }
}

void DescriptorImages::compile(Context& context)
{
    // check if we have already compiled the imageData.
    if ((_imageInfos.size() == _imageDataList.size() && _imageInfos.size() != 0) || _samplerImages.empty()) return;

    if (!_samplerImages.empty())
    {
        _imageDataList.clear();
        _imageDataList.reserve(_samplerImages.size());
        for(auto& samplerImage : _samplerImages)
        {
            samplerImage.first->compile(context);
            _imageDataList.emplace_back(vsg::transferImageData(context, samplerImage.second, samplerImage.first));
        }
    }

    // convert from VSG to Vk
    _imageInfos.resize(_imageDataList.size());
    for (size_t i = 0; i < _imageDataList.size(); ++i)
    {
        const ImageData& data = _imageDataList[i];
        VkDescriptorImageInfo& info = _imageInfos[i];
        if (data._sampler) info.sampler = *(data._sampler);
        else info.sampler = 0;

        if (data._imageView) info.imageView = *(data._imageView);
        else info.imageView = 0;

        info.imageLayout = data._imageLayout;
    }
}

bool DescriptorImages::assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
{
    Descriptor::assignTo(wds, descriptorSet);
    wds.descriptorCount = static_cast<uint32_t>(_imageInfos.size());
    wds.pImageInfo = _imageInfos.data();
    return true;
}

uint32_t DescriptorImages::getNumDescriptors() const
{
    return static_cast<uint32_t>(std::max(_imageDataList.size(), _samplerImages.size()));
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorBuffer
//
void DescriptorBuffer::copyDataListToBuffers()
{
    vsg::copyDataListToBuffers(_bufferDataList);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Uniform
//
Uniform::Uniform() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
{
}

void Uniform::read(Input& input)
{
    Descriptor::read(input);

    _dataList.resize(input.readValue<uint32_t>("NumData"));
    for (auto& data : _dataList)
    {
        data = input.readObject<Data>("Data");
    }
}

void Uniform::write(Output& output) const
{
    Descriptor::write(output);

    output.writeValue<uint32_t>("NumData", _dataList.size());
    for (auto& data : _dataList)
    {
        output.writeObject("Data", data.get());
    }
}

void Uniform::compile(Context& context)
{
    if (_implementation) return;

    auto bufferDataList = vsg::createHostVisibleBuffer(context.device, _dataList, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    vsg::copyDataListToBuffers(bufferDataList);
    _implementation = vsg::DescriptorBuffer::create(_dstBinding, _dstArrayElement, _descriptorType, bufferDataList);
}

bool Uniform::assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
{
    if (_implementation)
        return _implementation->assignTo(wds, descriptorSet);
    else
        return false;
}

void Uniform::copyDataListToBuffers()
{
    if (_implementation) _implementation->copyDataListToBuffers();
}
