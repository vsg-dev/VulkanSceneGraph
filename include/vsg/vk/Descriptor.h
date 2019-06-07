#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BufferData.h>
#include <vsg/vk/BufferView.h>
#include <vsg/vk/ImageData.h>

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

        virtual uint32_t getNumDescriptors() const { return 1; }
    };

    using Descriptors = std::vector<vsg::ref_ptr<vsg::Descriptor>>;

    using SamplerImage = std::pair<ref_ptr<Sampler>, ref_ptr<Data>>;
    using SamplerImages = std::vector<SamplerImage>;

    class VSG_DECLSPEC DescriptorImage : public Inherit<Descriptor, DescriptorImage>
    {
    public:
        DescriptorImage();

        DescriptorImage(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, ref_ptr<Sampler> sampler, ref_ptr<Data> image);

        DescriptorImage(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const SamplerImage& samplerImage);

        Sampler* getSampler() { return _samplerImage.first; }
        const Sampler* getSampler() const { return _samplerImage.first; }

        Data* getImage() { return _samplerImage.second; }
        const Data* getImage() const { return _samplerImage.second; }

        SamplerImage& getSamplerImage() { return _samplerImage; }
        const SamplerImage& getSamplerImage() const { return _samplerImage; }


        /** ImageData is automatically filled in by the DecriptorImage::compile() using the sampler and image data objects.*/
        ImageData& getImageData() { return _imageData; }
        const ImageData& getImageData() const { return _imageData; }

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context) override;

        bool assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const override;

        uint32_t getNumDescriptors() const override { return 1; }

    protected:
        SamplerImage _samplerImage;

        ImageData _imageData;
        VkDescriptorImageInfo _imageInfo;
    };
    VSG_type_name(vsg::DescriptorImage)

    class VSG_DECLSPEC DescriptorImages : public Inherit<Descriptor, DescriptorImages>
    {
    public:
        DescriptorImages();

        DescriptorImages(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const SamplerImages& sampleImages);


        SamplerImages& getSamplerImages() { return _samplerImages; }
        const SamplerImages& getSamplerImages() const { return _samplerImages; }

        /** ImageDataList is automatically filled in by the DecriptorImage::compile() using the sampler and image data objects.*/
        ImageDataList& getImageDataList() { return _imageDataList; }
        const ImageDataList& getImageDataList() const { return _imageDataList; }

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context) override;

        bool assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const override;

        uint32_t getNumDescriptors() const override;

    protected:
        SamplerImages _samplerImages;

        // populated by compile()
        ImageDataList _imageDataList;
        std::vector<VkDescriptorImageInfo> _imageInfos;
    };
    VSG_type_name(vsg::DescriptorImages)

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

        uint32_t getNumDescriptors() const override { return static_cast<uint32_t>(_bufferDataList.size()); }

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

        uint32_t getNumDescriptors() const override { return static_cast<uint32_t>(_texelBufferViewList.size()); }

    protected:
        BufferViewList _texelBufferViewList;
        std::vector<VkBufferView> _texelBufferViews;
    };

    class VSG_DECLSPEC Uniform : public Inherit<Descriptor, Uniform>
    {
    public:
        Uniform();

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context) override;

        bool assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const override;

        void copyDataListToBuffers();

        uint32_t getNumDescriptors() const override { return static_cast<uint32_t>(_dataList.size()); }

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
