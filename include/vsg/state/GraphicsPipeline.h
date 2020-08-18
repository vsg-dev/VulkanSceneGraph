#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/GraphicsPipelineStates.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/state/StateCommand.h>
#include <vsg/vk/RenderPass.h>

namespace vsg
{

    class VSG_DECLSPEC GraphicsPipeline : public Inherit<Object, GraphicsPipeline>
    {
    public:
        GraphicsPipeline();

        GraphicsPipeline(PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const GraphicsPipelineStates& pipelineStates, uint32_t subpass = 0);

        /// VkGraphicsPipelineCreateInfo settings
        ShaderStages stages;
        GraphicsPipelineStates pipelineStates;
        ref_ptr<PipelineLayout> layout;
        ref_ptr<RenderPass> renderPass;
        uint32_t subpass;

        void read(Input& input) override;
        void write(Output& output) const override;

#ifdef DEPRECATED_API
        PipelineLayout* getPipelineLayout() { return layout; }
        const PipelineLayout* getPipelineLayout() const { return layout; }

        ShaderStages& getShaderStages() { return stages; }
        const ShaderStages& getShaderStages() const { return stages; }

        GraphicsPipelineStates& getPipelineStates() { return pipelineStates; }
        const GraphicsPipelineStates& getPipelineStates() const { return pipelineStates; }

        uint32_t& getSubpass() { return subpass; }
        uint32_t getSubpass() const { return subpass; }
#endif
        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release(uint32_t deviceID) { _implementation[deviceID] = {}; }
        void release() { _implementation.clear(); }

        VkPipeline vk(uint32_t deviceID) const { return _implementation[deviceID]->_pipeline; }

    protected:
        virtual ~GraphicsPipeline();

        struct Implementation : public Inherit<Object, Implementation>
        {
            Implementation(Context& context, Device* device, RenderPass* renderPass, PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const GraphicsPipelineStates& pipelineStates, uint32_t subpass);

            virtual ~Implementation();

            VkPipeline _pipeline;

            // TODO need to convert to use Implementation versions of RenderPass and PipelineLayout
            ref_ptr<Device> _device;
        };

        vk_buffer<ref_ptr<Implementation>> _implementation;

    };
    VSG_type_name(vsg::GraphicsPipeline);

    /// Encapsulation for vkCmdBindPipeline
    class VSG_DECLSPEC BindGraphicsPipeline : public Inherit<StateCommand, BindGraphicsPipeline>
    {
    public:
        BindGraphicsPipeline(GraphicsPipeline* in_pipeline = nullptr);

        /// pipeline to pass in the vkCmdBindPipeline call;
        ref_ptr<GraphicsPipeline> pipeline;

        void read(Input& input) override;
        void write(Output& output) const override;

#ifdef DEPRECATED_API
        void setPipeline(GraphicsPipeline* in_pipeline) { pipeline = in_pipeline; }
        GraphicsPipeline* getPipeline() { return pipeline; }
        const GraphicsPipeline* getPipeline() const { return pipeline; }
#endif
        void record(CommandBuffer& commandBuffer) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context) override;

        virtual void release();

    public:
        virtual ~BindGraphicsPipeline();

    };
    VSG_type_name(vsg::BindGraphicsPipeline);

} // namespace vsg
