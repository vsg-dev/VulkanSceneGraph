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
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
{
}

DescriptorImage::DescriptorImage(ref_ptr<Sampler> sampler, ref_ptr<Data> image, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType)
{
    if (sampler || image) _samplerImages.emplace_back(SamplerImage(sampler, image));
}

DescriptorImage::DescriptorImage(const SamplerImage& samplerImage, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType)
{
    if (samplerImage.first || samplerImage.second) _samplerImages.emplace_back(samplerImage);
}

DescriptorImage::DescriptorImage(const SamplerImages& samplerImages, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType):
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
        samplerImage.first = input.readObject<Sampler>("Sampler");
        samplerImage.second = input.readObject<Data>("Image");
    }
}

void DescriptorImage::write(Output& output) const
{
    Descriptor::write(output);

    output.writeValue<uint32_t>("NumImages", _samplerImages.size());
    for (auto& samplerImage : _samplerImages)
    {
        output.writeObject("Sampler", samplerImage.first.get());
        output.writeObject("Image", samplerImage.second.get());
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

/////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorBuffer
//
DescriptorBuffer::DescriptorBuffer() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
{
}

DescriptorBuffer::DescriptorBuffer(ref_ptr<Data> data, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType)
{
    if (data) _dataList.emplace_back(data);
}

DescriptorBuffer::DescriptorBuffer(const DataList& dataList, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType),
    _dataList(dataList)
{
}

DescriptorBuffer::DescriptorBuffer(const BufferDataList& bufferDataList, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType),
    _bufferDataList(bufferDataList)
{
}

void DescriptorBuffer::read(Input& input)
{
    _bufferDataList.clear();
    _bufferInfos.clear();

    Descriptor::read(input);

    _dataList.resize(input.readValue<uint32_t>("NumData"));
    for (auto& data : _dataList)
    {
        data = input.readObject<Data>("Data");
    }
}

void DescriptorBuffer::write(Output& output) const
{
    Descriptor::write(output);

    output.writeValue<uint32_t>("NumData", _dataList.size());
    for (auto& data : _dataList)
    {
        output.writeObject("Data", data.get());
    }
}

void DescriptorBuffer::compile(Context& context)
{
    // check if already compiled
    if ((_bufferInfos.size() >= _bufferDataList.size()) && (_bufferInfos.size() >= _dataList.size())) return;


    if (_bufferDataList.size()<_dataList.size())
    {
        VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
#if 1
        _bufferDataList = vsg::createHostVisibleBuffer(context.device, _dataList, bufferUsageFlags, VK_SHARING_MODE_EXCLUSIVE);
        vsg::copyDataListToBuffers(_bufferDataList);
#else
        _bufferDataList = vsg::createBufferAndTransferData(context, _dataList, bufferUsageFlags, VK_SHARING_MODE_EXCLUSIVE);
#endif
    }

    // convert from VSG to Vk
    _bufferInfos.resize(_bufferDataList.size());
    for (size_t i = 0; i < _bufferDataList.size(); ++i)
    {
        const BufferData& data = _bufferDataList[i];
        VkDescriptorBufferInfo& info = _bufferInfos[i];
        info.buffer = *(data._buffer);
        info.offset = data._offset;
        info.range = data._range;
    }
}

bool DescriptorBuffer::assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
{
    Descriptor::assignTo(wds, descriptorSet);
    wds.descriptorCount = static_cast<uint32_t>(_bufferInfos.size());
    wds.pBufferInfo = _bufferInfos.data();
    return true;
}

uint32_t DescriptorBuffer::getNumDescriptors() const
{
    return static_cast<uint32_t>(std::max(_bufferDataList.size(), _dataList.size()));
}

void DescriptorBuffer::copyDataListToBuffers()
{
    vsg::copyDataListToBuffers(_bufferDataList);
}
