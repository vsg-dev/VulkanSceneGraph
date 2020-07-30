#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/maths/vec4.h>

namespace vsg
{
    // forward declare
    class Context;

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

    class VSG_DECLSPEC VertexInputState : public Inherit<GraphicsPipelineState, VertexInputState>
    {
    public:
        using Bindings = std::vector<VkVertexInputBindingDescription>;
        using Attributes = std::vector<VkVertexInputAttributeDescription>;

        VertexInputState();
        VertexInputState(const Bindings& bindings, const Attributes& attributes);

        Bindings vertexBindingDescriptions;
        Attributes vertexAttributeDescriptions;

        void read(Input& input) override;
        void write(Output& output) const override;
        void apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~VertexInputState();
    };
    VSG_type_name(vsg::VertexInputState);

    class VSG_DECLSPEC InputAssemblyState : public Inherit<GraphicsPipelineState, InputAssemblyState>
    {
    public:
        InputAssemblyState();
        InputAssemblyState(VkPrimitiveTopology primitiveTopology, VkBool32 primitiveRestart = VK_FALSE);

        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkBool32 primitiveRestartEnable = VK_FALSE;

        void read(Input& input) override;
        void write(Output& output) const override;
        void apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~InputAssemblyState();
    };
    VSG_type_name(vsg::InputAssemblyState);

    class VSG_DECLSPEC TessellationState : public Inherit<GraphicsPipelineState, TessellationState>
    {
    public:
        TessellationState(uint32_t in_patchControlPoints = 1);

        // patchControlPoints must be greater than zero and less than or equal to VkPhysicalDeviceLimits::maxTessellationPatchSize
        uint32_t patchControlPoints = 1;

        void read(Input& input) override;
        void write(Output& output) const override;

        void apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~TessellationState();
    };
    VSG_type_name(vsg::TessellationState);

    class VSG_DECLSPEC ViewportState : public Inherit<GraphicsPipelineState, ViewportState>
    {
    public:
        ViewportState();

        /// Create ViewportState containing a single Viewport and Scissor pair with specified extent
        explicit ViewportState(const VkExtent2D& extent);

        using Viewports = std::vector<VkViewport>;
        using Scissors = std::vector<VkRect2D>;

        Viewports viewports;
        Scissors scissors;

        /// set to a single Viewport and Scissor pair with specified extent
        void set(const VkExtent2D& extent);

        /// get or create the first Viewport
        VkViewport& getViewport();

        /// get or create the first Scissor
        VkRect2D& getScissor();

        void read(Input& input) override;
        void write(Output& output) const override;
        void apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~ViewportState();
    };
    VSG_type_name(vsg::ViewportState);

    class VSG_DECLSPEC RasterizationState : public Inherit<GraphicsPipelineState, RasterizationState>
    {
    public:
        RasterizationState();

        VkBool32 depthClampEnable = VK_FALSE;
        VkBool32 rasterizerDiscardEnable = VK_FALSE;
        VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        VkBool32 depthBiasEnable = VK_FALSE;
        float depthBiasConstantFactor = 1.0f;
        float depthBiasClamp = 0.0f;
        float depthBiasSlopeFactor = 1.0f;
        float lineWidth = 1.0f;

        void read(Input& input) override;
        void write(Output& output) const override;
        void apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~RasterizationState();
    };
    VSG_type_name(vsg::RasterizationState);

    class VSG_DECLSPEC MultisampleState : public Inherit<GraphicsPipelineState, MultisampleState>
    {
    public:
        MultisampleState(VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT);

        VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        VkBool32 sampleShadingEnable = VK_FALSE;
        float minSampleShading = 0.0f;
        std::vector<VkSampleMask> sampleMasks;
        VkBool32 alphaToCoverageEnable = VK_FALSE;
        VkBool32 alphaToOneEnable = VK_FALSE;

        void read(Input& input) override;
        void write(Output& output) const override;
        void apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~MultisampleState();
    };
    VSG_type_name(vsg::MultisampleState);

    class VSG_DECLSPEC DepthStencilState : public Inherit<GraphicsPipelineState, DepthStencilState>
    {
    public:
        DepthStencilState();

        VkBool32 depthTestEnable = VK_TRUE;
        VkBool32 depthWriteEnable = VK_TRUE;
        VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
        VkBool32 depthBoundsTestEnable = VK_FALSE;
        VkBool32 stencilTestEnable = VK_FALSE;
        VkStencilOpState front = {};
        VkStencilOpState back = {};
        float minDepthBounds = 0.0f;
        float maxDepthBounds = 1.0f;

        void read(Input& input) override;
        void write(Output& output) const override;
        void apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~DepthStencilState();
    };
    VSG_type_name(vsg::DepthStencilState);

    class VSG_DECLSPEC ColorBlendState : public Inherit<GraphicsPipelineState, ColorBlendState>
    {
    public:
        using ColorBlendAttachments = std::vector<VkPipelineColorBlendAttachmentState>;

        ColorBlendState();
        ColorBlendState(const ColorBlendAttachments& colorBlendAttachments);

        VkBool32 logicOpEnable = VK_FALSE;
        VkLogicOp logicOp = VK_LOGIC_OP_COPY;
        ColorBlendAttachments attachments;
        float blendConstants[4] = {0.0f, 0.0f, 0.0f, 0.0f};

        void read(Input& input) override;
        void write(Output& output) const override;
        void apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~ColorBlendState();

        ColorBlendAttachments _colorBlendAttachments;
    };
    VSG_type_name(vsg::ColorBlendState);

    class VSG_DECLSPEC DynamicState : public Inherit<GraphicsPipelineState, DynamicState>
    {
    public:
        using DynamicStates = std::vector<VkDynamicState>;

        DynamicState();

        DynamicState(const DynamicStates& states) :
            dynamicStates(states) {}

        template<typename ...Args>
        DynamicState(Args... args) :
            dynamicStates({args...}) {}

        DynamicStates dynamicStates;

        void read(Input& input) override;
        void write(Output& output) const override;
        void apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~DynamicState();
    };
    VSG_type_name(vsg::DynamicState);

} // namespace vsg
