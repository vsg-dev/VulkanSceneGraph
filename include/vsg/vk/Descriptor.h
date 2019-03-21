#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BufferData.h>
#include <vsg/vk/BufferView.h>
#include <vsg/vk/ImageView.h>
#include <vsg/vk/Sampler.h>

namespace vsg
{
    // forward declare
    class Context;

    using DescriptorBufferInfos = std::vector<VkDescriptorBufferInfo>;

    class Descriptor : public Inherit<Object, Descriptor>
    {
    public:
        Descriptor(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
            _dstBinding(dstBinding),
            _dstArrayElement(dstArrayElement),
            _descriptorType(descriptorType)
        {
        }

        uint32_t _dstBinding;
        uint32_t _dstArrayElement;
        VkDescriptorType _descriptorType;

        void read(Input& input) override
        {
            Object::read(input);

            input.read("DstBinding", _dstBinding);
            input.read("DstArrayElement", _dstArrayElement);
        }

        void write(Output& output) const override
        {
            Object::write(output);

            output.write("DstBinding", _dstBinding);
            output.write("DstArrayElement", _dstArrayElement);
        }

        // compile the Vulkan object, context parameter used for Device
        virtual void compile(Context& /*context*/) {}

        virtual bool assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
        {
            wds = {};
            wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            wds.dstSet = descriptorSet;
            wds.dstBinding = _dstBinding;
            wds.dstArrayElement = _dstArrayElement;
            wds.descriptorType = _descriptorType;

            return false;
        }
    };

    using Descriptors = std::vector<vsg::ref_ptr<vsg::Descriptor>>;

    class ImageData
    {
    public:
        ImageData() :
            _imageLayout(VK_IMAGE_LAYOUT_UNDEFINED) {}

        ImageData(const ImageData& id) :
            _sampler(id._sampler),
            _imageView(id._imageView),
            _imageLayout(id._imageLayout) {}

        ImageData(Sampler* sampler, ImageView* imageView, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED) :
            _sampler(sampler),
            _imageView(imageView),
            _imageLayout(imageLayout) {}

        ImageData& operator=(const ImageData& rhs)
        {
            _sampler = rhs._sampler;
            _imageView = rhs._imageView;
            _imageLayout = rhs._imageLayout;
            return *this;
        }

        explicit operator bool() const { return _sampler.valid() && _imageView.valid(); }

        bool valid() const { return _sampler.valid() && _imageView.valid(); }

        ref_ptr<Sampler> _sampler;
        ref_ptr<ImageView> _imageView;
        VkImageLayout _imageLayout;
    };

    /// transfer Data to graphics memory, returning ImageData configuration.
    //extern VSG_DECLSPEC vsg::ImageData transferImageData(Device* device, CommandPool* commandPool, VkQueue queue, const Data* data, Sampler* sampler = nullptr);

    /// transfer Data to graphics memory, returning ImageData configuration.
    extern VSG_DECLSPEC vsg::ImageData transferImageData(Context& context, const Data* data, Sampler* sampler = nullptr);

    using ImageDataList = std::vector<ImageData>;

    class VSG_DECLSPEC DescriptorImage : public Inherit<Descriptor, DescriptorImage>
    {
    public:
        DescriptorImage(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const ImageDataList& imageDataList) :
            Inherit(dstBinding, dstArrayElement, descriptorType),
            _imageDataList(imageDataList)
        {
            // convert from VSG to Vk
            _imageInfos.resize(_imageDataList.size());
            for (size_t i = 0; i < _imageDataList.size(); ++i)
            {
                const ImageData& data = _imageDataList[i];
                VkDescriptorImageInfo& info = _imageInfos[i];
                info.sampler = *(data._sampler);
                info.imageView = *(data._imageView);
                info.imageLayout = data._imageLayout;
            }
        }

        virtual bool assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
        {
            Descriptor::assignTo(wds, descriptorSet);
            wds.descriptorCount = static_cast<uint32_t>(_imageInfos.size());
            wds.pImageInfo = _imageInfos.data();
            return true;
        }

    protected:
        ImageDataList _imageDataList;
        std::vector<VkDescriptorImageInfo> _imageInfos;
    };

    class VSG_DECLSPEC DescriptorBuffer : public Inherit<Descriptor, DescriptorBuffer>
    {
    public:
        DescriptorBuffer(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const BufferDataList& bufferDataList) :
            Inherit(dstBinding, dstArrayElement, descriptorType),
            _bufferDataList(bufferDataList)
        {
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

        virtual bool assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
        {
            Descriptor::assignTo(wds, descriptorSet);
            wds.descriptorCount = static_cast<uint32_t>(_bufferInfos.size());
            wds.pBufferInfo = _bufferInfos.data();
            return true;
        }

        void copyDataListToBuffers();

    protected:
        BufferDataList _bufferDataList;
        std::vector<VkDescriptorBufferInfo> _bufferInfos;
    };

    using BufferViewList = std::vector<ref_ptr<BufferView>>;

    class VSG_DECLSPEC DescriptorTexelBufferView : public Inherit<Descriptor, DescriptorTexelBufferView>
    {
    public:
        DescriptorTexelBufferView(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const BufferViewList& texelBufferViews) :
            Inherit(dstBinding, dstArrayElement, descriptorType),
            _texelBufferViewList(texelBufferViews)
        {
            _texelBufferViews.resize(_texelBufferViewList.size());
            for (size_t i = 0; i < _texelBufferViewList.size(); ++i)
            {
                _texelBufferViews[i] = *(_texelBufferViewList[i]);
            }
        }

        virtual bool assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
        {
            std::vector<VkBufferView> texelBufferViews(_texelBufferViewList.size());

            Descriptor::assignTo(wds, descriptorSet);
            wds.descriptorCount = static_cast<uint32_t>(_texelBufferViews.size());
            wds.pTexelBufferView = _texelBufferViews.data();
            return true;
        }

    protected:
        BufferViewList _texelBufferViewList;
        std::vector<VkBufferView> _texelBufferViews;
    };

    class VSG_DECLSPEC Texture : public Inherit<Descriptor, Texture>
    {
    public:
        Texture();

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context) override;

        bool assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const override;

        // settings
        VkSamplerCreateInfo _samplerInfo;
        ref_ptr<Data> _textureData;

        ref_ptr<vsg::Descriptor> _implementation;
    };
    VSG_type_name(vsg::Texture)

    class VSG_DECLSPEC Uniform : public Inherit<Descriptor, Uniform>
    {
    public:
        Uniform();

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context) override;

        bool assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const override;

        void copyDataListToBuffers();

        // settings
        DataList _dataList;

        ref_ptr<vsg::DescriptorBuffer> _implementation;
    };
    VSG_type_name(vsg::Uniform)

    struct Material
    {
        vec4 ambientColor;
        vec4 diffuseColor;
        vec4 specularColor;
        float shine;
    };

    class VSG_DECLSPEC MaterialValue : public Inherit<Value<Material>, MaterialValue>
    {
    public:
        MaterialValue() {}

        void read(Input& input) override
        {
            value().ambientColor = input.readValue<vec4>("ambientColor");
            value().diffuseColor = input.readValue<vec4>("diffuseColor");
            value().specularColor = input.readValue<vec4>("specularColor");
            value().shine = input.readValue<float>("shine");
        }
        void write(Output& output) const override
        {
            output.writeValue<vec4>("ambientColor", value().ambientColor);
            output.writeValue<vec4>("diffuseColor", value().diffuseColor);
            output.writeValue<vec4>("specularColor", value().specularColor);
            output.writeValue<float>("shine", value().shine);
        }
    };
    VSG_type_name(vsg::MaterialValue)

} // namespace vsg
