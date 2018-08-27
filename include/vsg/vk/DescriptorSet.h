#pragma once

#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/DescriptorSetLayout.h>
#include <vsg/vk/PipelineLayout.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/nodes/StateGroup.h>

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

    class BindDescriptorSets : public StateComponent
    {
    public:

        BindDescriptorSets(VkPipelineBindPoint bindPoint, PipelineLayout* pipelineLayout, const DescriptorSets& descriptorSets) :
            _bindPoint(bindPoint),
            _pipelineLayout(pipelineLayout),
            _descriptorSets(descriptorSets)
        {
            update();
        }

        virtual void pushTo(State& state) override;
        virtual void popFrom(State& state) override;
        virtual void dispatch(CommandBuffer& commandBuffer) const override;

        void update()
        {
            _vkDescriptorSets.resize(_descriptorSets.size());
            for (size_t i=0; i<_descriptorSets.size(); ++i)
            {
                _vkDescriptorSets[i] = *(_descriptorSets[i]);
            }
        }


    protected:
        virtual ~BindDescriptorSets() {}

        using VkDescriptorSets = std::vector<VkDescriptorSet>;

        VkPipelineBindPoint         _bindPoint;
        ref_ptr<PipelineLayout>     _pipelineLayout;
        DescriptorSets              _descriptorSets;
        VkDescriptorSets            _vkDescriptorSets;
    };

}
