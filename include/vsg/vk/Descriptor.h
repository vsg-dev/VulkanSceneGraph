#pragma once

#include <vsg/vk/Buffer.h>
#include <vsg/vk/Sampler.h>
#include <vsg/vk/ImageView.h>
#include <vsg/vk/BufferView.h>

#include <iostream>

namespace vsg
{

    using DescriptorBufferInfos = std::vector<VkDescriptorBufferInfo>;

    class Descriptor : public Object
    {
    public:
        Descriptor(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType):
            _dstBinding(dstBinding),
            _dstArrayElement(dstArrayElement),
            _descriptorType(descriptorType)
        {
        }

        uint32_t            _dstBinding;
        uint32_t            _dstArrayElement;
        VkDescriptorType    _descriptorType;

        virtual void assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
        {
            wds = {};
            wds.sType =VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            wds.dstSet = descriptorSet;
            wds.dstBinding = _dstBinding;
            wds.dstArrayElement = _dstArrayElement;
            wds.descriptorType = _descriptorType;
        }
    };

    using Descriptors = std::vector<vsg::ref_ptr<vsg::Descriptor>>;

    class ImageData
    {
    public:

        ImageData() : _imageLayout(VK_IMAGE_LAYOUT_UNDEFINED) {}

        ImageData(const ImageData& id) :
            _sampler(id._sampler),
            _imageView(id._imageView),
            _imageLayout(id._imageLayout) {}

        ImageData(Sampler* sampler, ImageView* imageView, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED):
            _sampler(sampler),
            _imageView(imageView),
            _imageLayout(imageLayout) {}

        ImageData& operator = (const ImageData& rhs)
        {
            _sampler = rhs._sampler;
            _imageView = rhs._imageView;
            _imageLayout = rhs._imageLayout;
        }

        explicit operator bool() const { return _sampler.valid() && _imageView.valid(); }

        bool valid() const { return _sampler.valid() && _imageView.valid(); }

        ref_ptr<Sampler>    _sampler;
        ref_ptr<ImageView>  _imageView;
        VkImageLayout       _imageLayout;
    };

    using ImageDataList = std::vector<ImageData>;

    class DescriptorImage : public Descriptor
    {
    public:

        DescriptorImage(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const ImageDataList& imageDataList) :
            Descriptor(dstBinding, dstArrayElement, descriptorType),
            _imageDataList(imageDataList)
        {
            // convert from VSG to Vk
            _imageInfos.resize(_imageDataList.size());
            for (size_t i=0; i<_imageDataList.size(); ++i)
            {
                const ImageData& data = _imageDataList[i];
                VkDescriptorImageInfo& info = _imageInfos[i];
                info.sampler = *(data._sampler);
                info.imageView = *(data._imageView);
                info.imageLayout = data._imageLayout;
            }
        }

        virtual void assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
        {
            Descriptor::assignTo(wds, descriptorSet);
            wds.descriptorCount = _imageInfos.size();
            wds.pImageInfo = _imageInfos.data();
        }

    protected:

        ImageDataList                       _imageDataList;
        std::vector<VkDescriptorImageInfo>  _imageInfos;
    };


    class BufferData
    {
    public:
        BufferData(Buffer* buffer, VkDeviceSize offset, VkDeviceSize range):
            _buffer(buffer),
            _offset(offset),
            _range(range) {}

        ref_ptr<Buffer> _buffer;
        VkDeviceSize    _offset;
        VkDeviceSize    _range;
    };

    using BufferDataList = std::vector<BufferData>;

    class DescriptorBuffer : public Descriptor
    {
    public:

        DescriptorBuffer(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const BufferDataList& bufferDataList) :
            Descriptor(dstBinding, dstArrayElement, descriptorType),
            _bufferDataList(bufferDataList)
        {
            // convert from VSG to Vk
            _bufferInfos.resize(_bufferDataList.size());
            for (size_t i=0; i<_bufferDataList.size(); ++i)
            {
                const BufferData& data = _bufferDataList[i];
                VkDescriptorBufferInfo& info = _bufferInfos[i];
                info.buffer = *(data._buffer);
                info.offset = data._offset;
                info.range = data._range;
            }
        }

        virtual void assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
        {
            Descriptor::assignTo(wds, descriptorSet);
            wds.descriptorCount = _bufferInfos.size();
            wds.pBufferInfo = _bufferInfos.data();
        }

    protected:

        BufferDataList                      _bufferDataList;
        std::vector<VkDescriptorBufferInfo> _bufferInfos;
    };

    using BufferViewList = std::vector<ref_ptr<BufferView>>;

    class DescriptorTexelBufferView : public Descriptor
    {
    public:

        DescriptorTexelBufferView(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const BufferViewList& texelBufferViews) :
            Descriptor(dstBinding, dstArrayElement, descriptorType),
            _texelBufferViewList(texelBufferViews)
        {
            _texelBufferViews.resize(_texelBufferViewList.size());
            for (size_t i=0; i<_texelBufferViewList.size(); ++i)
            {
                _texelBufferViews[i] = *(_texelBufferViewList[i]);
            }
        }

        virtual void assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
        {
            std::vector<VkBufferView> texelBufferViews(_texelBufferViewList.size());

            Descriptor::assignTo(wds, descriptorSet);
            wds.descriptorCount = _texelBufferViews.size();
            wds.pTexelBufferView = _texelBufferViews.data();
        }

    protected:

        BufferViewList              _texelBufferViewList;
        std::vector<VkBufferView>   _texelBufferViews;
    };

}
