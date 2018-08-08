#pragma once

#include <vsg/vk/Command.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/DescriptorSetLayout.h>
#include <vsg/vk/PipelineLayout.h>

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

    class DescriptorImage : public Descriptor
    {
    public:

        using Images = std::vector<VkDescriptorImageInfo>;

        DescriptorImage(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const Images& images) :
            Descriptor(dstBinding, dstArrayElement, descriptorType),
            _images(images) {}

        virtual void assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
        {
            Descriptor::assignTo(wds, descriptorSet);
            wds.descriptorCount = _images.size();
            wds.pImageInfo = _images.data();
        }

    protected:

        Images _images;
    };


    class DescriptorBuffer : public Descriptor
    {
    public:

        using Buffers = std::vector<VkDescriptorBufferInfo>;

        DescriptorBuffer(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const Buffers& buffers) :
            Descriptor(dstBinding, dstArrayElement, descriptorType),
            _buffers(buffers) {}

        virtual void assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
        {
            Descriptor::assignTo(wds, descriptorSet);
            wds.descriptorCount = _buffers.size();
            wds.pBufferInfo = _buffers.data();
        }

    protected:

        Buffers _buffers;
    };

    class DescriptorTexelBufferView : public Descriptor
    {
    public:

        using TexelBufferViews = std::vector<VkBufferView>;

        DescriptorTexelBufferView(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const TexelBufferViews& texelBufferViews) :
            Descriptor(dstBinding, dstArrayElement, descriptorType),
            _texelBufferViews(texelBufferViews) {}

        virtual void assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
        {
            Descriptor::assignTo(wds, descriptorSet);
            wds.descriptorCount = _texelBufferViews.size();
            wds.pTexelBufferView = _texelBufferViews.data();
        }

    protected:

        TexelBufferViews _texelBufferViews;
    };

    class DescriptorSet : public vsg::Object
    {
    public:
        DescriptorSet(VkDescriptorSet descriptorSet, Device* device, DescriptorPool* descriptorPool, DescriptorSetLayout* descriptorSetLayout, const Descriptors& descriptors);

        using Result = vsg::Result<DescriptorSet, VkResult, VK_SUCCESS>;

        static Result create(Device* device, DescriptorPool* descriptorPool, DescriptorSetLayout* descriptorSetLayout, const Descriptors& descriptors);

        void assign(const Descriptors& descriptors);

        operator VkDescriptorSet () const { return _descriptorSet; }

    protected:
        virtual ~DescriptorSet();

        VkDescriptorSet                 _descriptorSet;
        ref_ptr<Device>                 _device;
        ref_ptr<DescriptorPool>         _descriptorPool;
        ref_ptr<DescriptorSetLayout>    _descriptorSetLayout;
        Descriptors                     _descriptors;

    };

    using DescriptorSets = std::vector<ref_ptr<DescriptorSet>>;

    class CmdBindDescriptorSets : public Command
    {
    public:

        CmdBindDescriptorSets(VkPipelineBindPoint bindPoint, PipelineLayout* pipelineLayout, const DescriptorSets& descriptorSets) :
            _bindPoint(bindPoint),
            _pipelineLayout(pipelineLayout),
            _descriptorSets(descriptorSets)
        {
            update();
        }

        virtual void dispatch(VkCommandBuffer commandBuffer) const
        {
            vkCmdBindDescriptorSets(commandBuffer, _bindPoint, *_pipelineLayout, 0, _vkDescriptorSets.size(), _vkDescriptorSets.data(), 0, nullptr);
        }

        void update()
        {
            _vkDescriptorSets.resize(_descriptorSets.size());
            for (size_t i=0; i<_descriptorSets.size(); ++i)
            {
                _vkDescriptorSets[i] = *(_descriptorSets[i]);
            }
        }


    protected:
        virtual ~CmdBindDescriptorSets() {}

        using VkDescriptorSets = std::vector<VkDescriptorSet>;

        VkPipelineBindPoint         _bindPoint;
        ref_ptr<PipelineLayout>     _pipelineLayout;
        DescriptorSets              _descriptorSets;
        VkDescriptorSets            _vkDescriptorSets;
    };

}
