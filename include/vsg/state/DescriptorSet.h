#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/StateCommand.h>
#include <vsg/vk/DescriptorPool.h>

namespace vsg
{

    class VSG_DECLSPEC DescriptorSet : public Inherit<Object, DescriptorSet>
    {
    public:
        DescriptorSet();
        DescriptorSet(ref_ptr<DescriptorSetLayout> in_descriptorSetLayout, const Descriptors& in_descriptors);

        /// VkDescriptorSetAllocateInfo settings
        ref_ptr<DescriptorSetLayout> setLayout;
        Descriptors descriptors;

        template<class N, class V>
        static void t_traverse(N& ds, V& visitor)
        {
            if (ds.setLayout) ds.setLayout->accept(visitor);
            for (auto& descriptor : ds.descriptors) descriptor->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release(uint32_t deviceID) { _implementation[deviceID] = {}; }
        void release() { _implementation.clear(); }

        VkDescriptorSet vk(uint32_t deviceID) const { return _implementation[deviceID]->_descriptorSet; }

    protected:
        virtual ~DescriptorSet();

        struct Implementation : public Inherit<Object, Implementation>
        {
            Implementation(Device* device, DescriptorPool* descriptorPool, DescriptorSetLayout* descriptorSetLayout);

            virtual ~Implementation();

            void assign(Context& context, const Descriptors& descriptors);

            VkDescriptorSet _descriptorSet;
            ref_ptr<Device> _device;
            ref_ptr<DescriptorPool> _descriptorPool;
            ref_ptr<DescriptorSetLayout> _descriptorSetLayout;
            Descriptors _descriptors;
        };

        vk_buffer<ref_ptr<Implementation>> _implementation;
    };
    VSG_type_name(vsg::DescriptorSet);

    using DescriptorSets = std::vector<ref_ptr<DescriptorSet>>;

    class VSG_DECLSPEC BindDescriptorSets : public Inherit<StateCommand, BindDescriptorSets>
    {
    public:
        BindDescriptorSets();

        BindDescriptorSets(VkPipelineBindPoint in_bindPoint, PipelineLayout* in_layout, uint32_t in_firstSet, const DescriptorSets& in_descriptorSets) :
            Inherit(1 + in_firstSet),
            pipelineBindPoint(in_bindPoint),
            layout(in_layout),
            firstSet(in_firstSet),
            descriptorSets(in_descriptorSets)
        {
        }

        BindDescriptorSets(VkPipelineBindPoint in_bindPoint, PipelineLayout* in_layout, const DescriptorSets& in_descriptorSets) :
            Inherit(1), // slot 1
            pipelineBindPoint(in_bindPoint),
            layout(in_layout),
            firstSet(0),
            descriptorSets(in_descriptorSets)
        {
        }

        /// vkCmdBindDescriptorSets settings
        VkPipelineBindPoint pipelineBindPoint;
        ref_ptr<PipelineLayout> layout;
        uint32_t firstSet;
        DescriptorSets descriptorSets;

        template<class N, class V>
        static void t_traverse(N& bds, V& visitor)
        {
            if (bds.layout) bds.layout->accept(visitor);
            for (auto& ds : bds.descriptorSets) ds->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context) override;

        void record(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~BindDescriptorSets() {}

        struct VulkanData
        {
            VkPipelineLayout _vkPipelineLayout = 0;
            std::vector<VkDescriptorSet> _vkDescriptorSets;
        };

        vk_buffer<VulkanData> _vulkanData;
    };
    VSG_type_name(vsg::BindDescriptorSets);

    class VSG_DECLSPEC BindDescriptorSet : public Inherit<StateCommand, BindDescriptorSet>
    {
    public:
        BindDescriptorSet();

        BindDescriptorSet(VkPipelineBindPoint in_bindPoint, PipelineLayout* in_pipelineLayout, uint32_t in_firstSet, DescriptorSet* in_descriptorSet) :
            Inherit(1 + in_firstSet),
            pipelineBindPoint(in_bindPoint),
            layout(in_pipelineLayout),
            firstSet(in_firstSet),
            descriptorSet(in_descriptorSet)
        {
        }

        BindDescriptorSet(VkPipelineBindPoint in_bindPoint, PipelineLayout* in_pipelineLayout, DescriptorSet* in_descriptorSet) :
            Inherit(1), // slot 1
            pipelineBindPoint(in_bindPoint),
            layout(in_pipelineLayout),
            firstSet(0),
            descriptorSet(in_descriptorSet)
        {
        }

        // vkCmdBindDescriptorSets settings
        VkPipelineBindPoint pipelineBindPoint;
        ref_ptr<PipelineLayout> layout;
        uint32_t firstSet;
        ref_ptr<DescriptorSet> descriptorSet;

        template<class N, class V>
        static void t_traverse(N& bds, V& visitor)
        {
            if (bds.layout) bds.layout->accept(visitor);
            if (bds.descriptorSet) bds.descriptorSet->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context) override;

        void record(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~BindDescriptorSet() {}

        struct VulkanData
        {
            VkPipelineLayout _vkPipelineLayout = 0;
            VkDescriptorSet _vkDescriptorSet;
        };

        vk_buffer<VulkanData> _vulkanData;
    };
    VSG_type_name(vsg::BindDescriptorSet);

} // namespace vsg
