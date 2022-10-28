#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/vk/vk_buffer.h>

namespace vsg
{
    // forward declare
    class Context;

    using PushConstantRanges = std::vector<VkPushConstantRange>;

    /// PipelineLayout encapsulates VkPipelineLayout and the VkPipelineLayoutCreateInfo settings used to set it up.
    class VSG_DECLSPEC PipelineLayout : public Inherit<Object, PipelineLayout>
    {
    public:
        PipelineLayout();
        PipelineLayout(const DescriptorSetLayouts& in_setLayouts, const PushConstantRanges& in_pushConstantRanges, VkPipelineLayoutCreateFlags in_flags = 0);

        /// VkPipelineLayoutCreateInfo settings
        VkPipelineLayoutCreateFlags flags = 0;
        DescriptorSetLayouts setLayouts;
        PushConstantRanges pushConstantRanges;

        /// Vulkan VkPipelineLayout handle
        VkPipelineLayout vk(uint32_t deviceID) const { return _implementation[deviceID]->_pipelineLayout; }

        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context);

        void release(uint32_t deviceID) { _implementation[deviceID] = {}; }
        void release() { _implementation.clear(); }

    protected:
        virtual ~PipelineLayout();

        struct Implementation : public Inherit<Object, Implementation>
        {
            Implementation(Device* device, const DescriptorSetLayouts& descriptorSetLayouts, const PushConstantRanges& pushConstantRanges, VkPipelineLayoutCreateFlags flags = 0);

            virtual ~Implementation();

            VkPipelineLayout _pipelineLayout;

            ref_ptr<Device> _device;
        };

        vk_buffer<ref_ptr<Implementation>> _implementation;
    };
    VSG_type_name(vsg::PipelineLayout);

} // namespace vsg
