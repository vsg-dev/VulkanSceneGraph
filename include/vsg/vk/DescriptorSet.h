#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/DescriptorSetLayout.h>
#include <vsg/vk/PipelineLayout.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/nodes/StateGroup.h>

namespace vsg
{

    class VSG_EXPORT DescriptorSet : public vsg::Object
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

    class VSG_EXPORT BindDescriptorSets : public Inherit<StateComponent, BindDescriptorSets>
    {
    public:

        BindDescriptorSets(VkPipelineBindPoint bindPoint, PipelineLayout* pipelineLayout, const DescriptorSets& descriptorSets) :
            _bindPoint(bindPoint),
            _pipelineLayout(pipelineLayout),
            _descriptorSets(descriptorSets)
        {
            update();
        }

        void pushTo(State& state) override;
        void popFrom(State& state) override;
        void dispatch(CommandBuffer& commandBuffer) const override;

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
