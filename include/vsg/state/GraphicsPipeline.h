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
#include <vsg/vk/RenderPass.h>

namespace vsg
{
    // forward declare
    class Context;

    /// Base class for setting up the various pipeline states with the VkGraphicsPipelineCreateInfo
    /// Subclasses are ColorBlendState, DepthStencilState, DynamicState, InputAssemblyState,
    /// MultisampleState, RasterizationState, TessellationState, VertexInputState and ViewportState.
    class VSG_DECLSPEC GraphicsPipelineState : public Inherit<Object, GraphicsPipelineState>
    {
    public:
        GraphicsPipelineState() {}

        virtual void apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const = 0;

    protected:
        virtual ~GraphicsPipelineState() {}
    };
    VSG_type_name(vsg::GraphicsPipelineState);

    using GraphicsPipelineStates = std::vector<ref_ptr<GraphicsPipelineState>>;

    /// GraphicsPipeline encapsulates graphics VkPipeline and the VkGraphicsPipelineCreateInfo settings used to set it up.
    class VSG_DECLSPEC GraphicsPipeline : public Inherit<Object, GraphicsPipeline>
    {
    public:
        GraphicsPipeline();

        GraphicsPipeline(PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const GraphicsPipelineStates& pipelineStates, uint32_t subpass = 0);

        VkPipeline vk(uint32_t deviceID) const { return _implementation[deviceID]->_pipeline; }

        /// VkGraphicsPipelineCreateInfo settings
        ShaderStages stages;
        GraphicsPipelineStates pipelineStates;
        ref_ptr<PipelineLayout> layout;
        ref_ptr<RenderPass> renderPass;
        uint32_t subpass;

        int compare(const Object& rhs_object) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release(uint32_t viewID) { _implementation[viewID] = {}; }
        void release() { _implementation.clear(); }

    protected:
        virtual ~GraphicsPipeline();

        struct Implementation : public Inherit<Object, Implementation>
        {
            Implementation(Context& context, Device* device, const RenderPass* renderPass, const PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const GraphicsPipelineStates& pipelineStates, uint32_t subpass);

            virtual ~Implementation();

            VkPipeline _pipeline;

            ref_ptr<Device> _device;
        };

        std::vector<ref_ptr<Implementation>> _implementation;
    };
    VSG_type_name(vsg::GraphicsPipeline);

    /// BindGraphicsPipeline state command encapsulates the vkCmdBindPipeline call for a GraphicsPipeline.
    class VSG_DECLSPEC BindGraphicsPipeline : public Inherit<StateCommand, BindGraphicsPipeline>
    {
    public:
        BindGraphicsPipeline(GraphicsPipeline* in_pipeline = nullptr);

        /// pipeline to pass in the vkCmdBindPipeline call;
        ref_ptr<GraphicsPipeline> pipeline;

        int compare(const Object& rhs_object) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        void record(CommandBuffer& commandBuffer) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context) override;

        virtual void release();

    public:
        virtual ~BindGraphicsPipeline();
    };
    VSG_type_name(vsg::BindGraphicsPipeline);

} // namespace vsg
