#pragma once

#include <vsg/vk/Command.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/DescriptorSetLayout.h>
#include <vsg/vk/PipelineLayout.h>
#include <vsg/vk/Descriptor.h>

namespace vsg
{

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

        virtual void dispatch(CommandBuffer& commandBuffer) const
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
