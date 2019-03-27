#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Command.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/DescriptorSetLayout.h>
#include <vsg/vk/PipelineLayout.h>

namespace vsg
{

    class VSG_DECLSPEC DescriptorSet : public Inherit<Object, DescriptorSet>
    {
    public:
        DescriptorSet();
        DescriptorSet(const DescriptorSetLayouts& descriptorSetLayouts, const Descriptors& descriptors);

        template<class N, class V>
        static void t_traverse(N& ds, V& visitor)
        {
            for (auto& dsl : ds._descriptorSetLayouts) dsl->accept(visitor);
            for (auto& descriptor : ds._descriptors) descriptor->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        const DescriptorSetLayouts& getDescriptorSetLayouts() const { return _descriptorSetLayouts; }
        const Descriptors& getDescriptors() const { return _descriptors; }

        class VSG_DECLSPEC Implementation : public Inherit<Object, Implementation>
        {
        public:
            Implementation(VkDescriptorSet descriptorSet, Device* device, DescriptorPool* descriptorPool, const DescriptorSetLayouts& descriptorSetLayouts, const Descriptors& descriptors);

            using Result = vsg::Result<Implementation, VkResult, VK_SUCCESS>;

            static Result create(Device* device, DescriptorPool* descriptorPool, const DescriptorSetLayouts& descriptorSetLayouts, const Descriptors& descriptors);

            void assign(const Descriptors& descriptors);

            operator VkDescriptorSet() const { return _descriptorSet; }

        protected:
            virtual ~Implementation();

            VkDescriptorSet _descriptorSet;
            ref_ptr<Device> _device;
            ref_ptr<DescriptorPool> _descriptorPool;
            DescriptorSetLayouts _descriptorSetLayouts;
            Descriptors _descriptors;
        };

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release() { _implementation = nullptr; }

        operator VkDescriptorSet() const { return *_implementation; }

        Implementation* implementation() { return _implementation; }
        const Implementation* implementation() const { return _implementation; }

    protected:
        virtual ~DescriptorSet();

        DescriptorSetLayouts _descriptorSetLayouts;
        Descriptors _descriptors;

        ref_ptr<Implementation> _implementation;
    };
    VSG_type_name(vsg::DescriptorSet);

    using DescriptorSets = std::vector<ref_ptr<DescriptorSet>>;

    class VSG_DECLSPEC BindDescriptorSets : public Inherit<StateCommand, BindDescriptorSets>
    {
    public:
        BindDescriptorSets();

        BindDescriptorSets(VkPipelineBindPoint bindPoint, PipelineLayout* pipelineLayout, uint32_t firstSet, const DescriptorSets& descriptorSets) :
            _bindPoint(bindPoint),
            _pipelineLayout(pipelineLayout),
            _firstSet(firstSet),
            _descriptorSets(descriptorSets)
        {
        }

        BindDescriptorSets(VkPipelineBindPoint bindPoint, PipelineLayout* pipelineLayout, const DescriptorSets& descriptorSets) :
            _bindPoint(bindPoint),
            _pipelineLayout(pipelineLayout),
            _firstSet(0),
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
        const PipelineLayout* getPipelineLayout() const { return _pipelineLayout; }
        uint32_t getFirstSet() { return _firstSet; }
        const DescriptorSets& getDescriptorSets() const { return _descriptorSets; }

        void pushTo(State& state) const override;
        void popFrom(State& state) const override;
        void dispatch(CommandBuffer& commandBuffer) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context) override;

    protected:
        virtual ~BindDescriptorSets() {}

        VkPipelineBindPoint _bindPoint;
        ref_ptr<PipelineLayout> _pipelineLayout;
        uint32_t _firstSet;
        DescriptorSets _descriptorSets;

        using VkDescriptorSets = std::vector<VkDescriptorSet>;
        VkDescriptorSets _vkDescriptorSets;
    };
    VSG_type_name(vsg::BindDescriptorSets);

} // namespace vsg
