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
        DescriptorSet(ref_ptr<DescriptorSetLayout> descriptorSetLayout, const Descriptors& descriptors);

        template<class N, class V>
        static void t_traverse(N& ds, V& visitor)
        {
            if (ds._descriptorSetLayout) ds._descriptorSetLayout->accept(visitor);
            for (auto& descriptor : ds._descriptors) descriptor->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        const DescriptorSetLayout* getDescriptorSetLayout() const { return _descriptorSetLayout; }
        const Descriptors& getDescriptors() const { return _descriptors; }

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

        ref_ptr<DescriptorSetLayout> _descriptorSetLayout;
        Descriptors _descriptors;
    };
    VSG_type_name(vsg::DescriptorSet);

    using DescriptorSets = std::vector<ref_ptr<DescriptorSet>>;

    class VSG_DECLSPEC BindDescriptorSets : public Inherit<StateCommand, BindDescriptorSets>
    {
    public:
        BindDescriptorSets();

        BindDescriptorSets(VkPipelineBindPoint bindPoint, PipelineLayout* pipelineLayout, uint32_t firstSet, const DescriptorSets& descriptorSets) :
            Inherit(1), // slot 1
            _bindPoint(bindPoint),
            _firstSet(firstSet),
            _pipelineLayout(pipelineLayout),
            _descriptorSets(descriptorSets)
        {
        }

        BindDescriptorSets(VkPipelineBindPoint bindPoint, PipelineLayout* pipelineLayout, const DescriptorSets& descriptorSets) :
            Inherit(1), // slot 1
            _bindPoint(bindPoint),
            _firstSet(0),
            _pipelineLayout(pipelineLayout),
            _descriptorSets(descriptorSets)
        {
        }

        template<class N, class V>
        static void t_traverse(N& bds, V& visitor)
        {
            if (bds._pipelineLayout) bds._pipelineLayout->accept(visitor);
            for (auto& ds : bds._descriptorSets) ds->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        VkPipelineBindPoint getBindPoint() { return _bindPoint; }
        PipelineLayout* getPipelineLayout() { return _pipelineLayout; }
        const PipelineLayout* getPipelineLayout() const { return _pipelineLayout; }
        uint32_t getFirstSet() { return _firstSet; }
        const DescriptorSets& getDescriptorSets() const { return _descriptorSets; }

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context) override;

        void dispatch(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~BindDescriptorSets() {}

        VkPipelineBindPoint _bindPoint;
        uint32_t _firstSet;

        struct VulkanData
        {
            VkPipelineLayout _vkPipelineLayout = 0;
            std::vector<VkDescriptorSet> _vkDescriptorSets;
        };

        vk_buffer<VulkanData> _vulkanData;

        ref_ptr<PipelineLayout> _pipelineLayout;
        DescriptorSets _descriptorSets;
    };
    VSG_type_name(vsg::BindDescriptorSets);

    class VSG_DECLSPEC BindDescriptorSet : public Inherit<StateCommand, BindDescriptorSet>
    {
    public:
        BindDescriptorSet();

        BindDescriptorSet(VkPipelineBindPoint bindPoint, PipelineLayout* pipelineLayout, uint32_t firstSet, DescriptorSet* descriptorSet) :
            Inherit(1), // slot 1
            _bindPoint(bindPoint),
            _firstSet(firstSet),
            _pipelineLayout(pipelineLayout),
            _descriptorSet(descriptorSet)
        {
        }

        BindDescriptorSet(VkPipelineBindPoint bindPoint, PipelineLayout* pipelineLayout, DescriptorSet* descriptorSet) :
            Inherit(1), // slot 1
            _bindPoint(bindPoint),
            _firstSet(0),
            _pipelineLayout(pipelineLayout),
            _descriptorSet(descriptorSet)
        {
        }

        template<class N, class V>
        static void t_traverse(N& bds, V& visitor)
        {
            if (bds._pipelineLayout) bds._pipelineLayout->accept(visitor);
            if (bds._descriptorSet) bds._descriptorSet->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        VkPipelineBindPoint getBindPoint() const { return _bindPoint; }
        PipelineLayout* getPipelineLayout() { return _pipelineLayout; }
        const PipelineLayout* getPipelineLayout() const { return _pipelineLayout; }
        uint32_t getFirstSet() const { return _firstSet; }
        DescriptorSet* getDescriptorSet() { return _descriptorSet; }
        const DescriptorSet* getDescriptorSet() const { return _descriptorSet; }

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context) override;

        void dispatch(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~BindDescriptorSet() {}

        VkPipelineBindPoint _bindPoint;
        uint32_t _firstSet;

        struct VulkanData
        {
            VkPipelineLayout _vkPipelineLayout = 0;
            VkDescriptorSet _vkDescriptorSet;
        };

        vk_buffer<VulkanData> _vulkanData;

        // settings
        ref_ptr<PipelineLayout> _pipelineLayout;
        ref_ptr<DescriptorSet> _descriptorSet;
    };
    VSG_type_name(vsg::BindDescriptorSet);

} // namespace vsg
