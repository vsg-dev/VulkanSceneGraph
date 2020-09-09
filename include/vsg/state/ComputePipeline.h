#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/PipelineLayout.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/state/StateCommand.h>

namespace vsg
{

    class VSG_DECLSPEC ComputePipeline : public Inherit<Object, ComputePipeline>
    {
    public:
        ComputePipeline();
        ComputePipeline(PipelineLayout* pipelineLayout, ShaderStage* shaderStage);

        void read(Input& input) override;
        void write(Output& output) const override;

        /// VkComputePipelineCreateInfo settings
        ref_ptr<PipelineLayout> layout;
        ref_ptr<ShaderStage> stage;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release(uint32_t deviceID) { _implementation[deviceID] = {}; }
        void release() { _implementation.clear(); }

        VkPipeline vk(uint32_t deviceID) const { return _implementation[deviceID]->_pipeline; }

    protected:
        virtual ~ComputePipeline();

        struct Implementation : public Inherit<Object, Implementation>
        {
            Implementation(Context& context, Device* device, PipelineLayout* pipelineLayout, ShaderStage* shaderStage);
            virtual ~Implementation();

            VkPipeline _pipeline;
            ref_ptr<Device> _device;
        };

        vk_buffer<ref_ptr<Implementation>> _implementation;
    };
    VSG_type_name(vsg::ComputePipeline);

    class VSG_DECLSPEC BindComputePipeline : public Inherit<StateCommand, BindComputePipeline>
    {
    public:
        BindComputePipeline(ComputePipeline* pipeline = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        /// pipeline to pass in the vkCmdBindPipeline call;
        ref_ptr<ComputePipeline> pipeline;

        void record(CommandBuffer& commandBuffer) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context) override;

    public:
        virtual ~BindComputePipeline();
    };
    VSG_type_name(vsg::BindComputePipeline);

} // namespace vsg
