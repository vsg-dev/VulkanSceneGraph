#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Command.h>
#include <vsg/vk/PipelineLayout.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/ShaderStage.h>

namespace vsg
{

    class GraphicsPipelineState : public Inherit<Object, GraphicsPipelineState>
    {
    public:
        GraphicsPipelineState() {}

        virtual VkStructureType getType() const = 0;

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const = 0;

    protected:
        virtual ~GraphicsPipelineState() {}
    };
    VSG_type_name(vsg::GraphicsPipelineState);

    using GraphicsPipelineStates = std::vector<ref_ptr<GraphicsPipelineState>>;

    class VSG_DECLSPEC GraphicsPipeline : public Inherit<Object, GraphicsPipeline>
    {
    public:
        GraphicsPipeline();

        GraphicsPipeline(PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const GraphicsPipelineStates& pipelineStates, uint32_t subpass = 0, AllocationCallbacks* allocator = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        PipelineLayout* getPipelineLayout() { return _pipelineLayout; }
        const PipelineLayout* getPipelineLayout() const { return _pipelineLayout; }

        ShaderStages& getShaderStages() { return _shaderStages; }
        const ShaderStages& getShaderStages() const { return _shaderStages; }

        GraphicsPipelineStates& getPipelineStates() { return _pipelineStates; }
        const GraphicsPipelineStates& getPipelineStates() const { return _pipelineStates; }

        uint32_t& getSubpass() { return _subpass; }
        uint32_t getSubpass() const { return _subpass; }

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
            Implementation(VkPipeline pipeline, Device* device, RenderPass* renderPass, PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const GraphicsPipelineStates& pipelineStates, AllocationCallbacks* allocator = nullptr);
            virtual ~Implementation();

            using Result = vsg::Result<Implementation, VkResult, VK_SUCCESS>;

            /** Create a GraphicsPipeline.*/
            static Result create(Device* device, RenderPass* renderPass, PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const GraphicsPipelineStates& pipelineStates, uint32_t subpass, AllocationCallbacks* allocator = nullptr);

            VkPipeline _pipeline;

            // TODO need to convert to use Implementation versions of RenderPass and PipelineLayout
            ref_ptr<Device> _device;
            ref_ptr<RenderPass> _renderPass;
            ref_ptr<PipelineLayout> _pipelineLayout;
            ShaderStages _shaderStages;
            GraphicsPipelineStates _pipelineStates;
            ref_ptr<AllocationCallbacks> _allocator;
        };

        vk_buffer<ref_ptr<Implementation>> _implementation;

        ref_ptr<RenderPass> _renderPass;
        ref_ptr<PipelineLayout> _pipelineLayout;
        ShaderStages _shaderStages;
        GraphicsPipelineStates _pipelineStates;
        uint32_t _subpass;
        ref_ptr<AllocationCallbacks> _allocator;
    };
    VSG_type_name(vsg::GraphicsPipeline);

    class VSG_DECLSPEC BindGraphicsPipeline : public Inherit<StateCommand, BindGraphicsPipeline>
    {
    public:
        BindGraphicsPipeline(GraphicsPipeline* pipeline = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        void setPipeline(GraphicsPipeline* pipeline) { _pipeline = pipeline; }
        GraphicsPipeline* getPipeline() { return _pipeline; }
        const GraphicsPipeline* getPipeline() const { return _pipeline; }

        void dispatch(CommandBuffer& commandBuffer) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context) override;

        virtual void release();

    public:
        virtual ~BindGraphicsPipeline();

        ref_ptr<GraphicsPipeline> _pipeline;
    };
    VSG_type_name(vsg::BindGraphicsPipeline);

    class VSG_DECLSPEC VertexInputState : public Inherit<GraphicsPipelineState, VertexInputState>, public VkPipelineVertexInputStateCreateInfo
    {
    public:
        using Bindings = std::vector<VkVertexInputBindingDescription>;
        using Attributes = std::vector<VkVertexInputAttributeDescription>;

        VertexInputState();
        VertexInputState(const Bindings& bindings, const Attributes& attributes);

        void read(Input& input) override;
        void write(Output& output) const override;

        VkStructureType getType() const override { return sType; }

        const Bindings& geBindings() { return _bindings; }

        const Attributes& getAttributes() const { return _attributes; }

        void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~VertexInputState();

        void _assign();

        Bindings _bindings;
        Attributes _attributes;
    };
    VSG_type_name(vsg::VertexInputState);

    class VSG_DECLSPEC InputAssemblyState : public Inherit<GraphicsPipelineState, InputAssemblyState>, public VkPipelineInputAssemblyStateCreateInfo
    {
    public:
        InputAssemblyState();
        InputAssemblyState(VkPrimitiveTopology primitiveTopology, bool enablePrimitiveRestart = false);

        void read(Input& input) override;
        void write(Output& output) const override;

        VkStructureType getType() const override { return sType; }

        void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~InputAssemblyState();
    };
    VSG_type_name(vsg::InputAssemblyState);

    class VSG_DECLSPEC ViewportState : public Inherit<GraphicsPipelineState, ViewportState>, public VkPipelineViewportStateCreateInfo
    {
    public:
        ViewportState();
        ViewportState(const VkExtent2D& extent);

        VkStructureType getType() const override { return sType; }

        VkViewport& getViewport() { return _viewport; }
        VkRect2D& getScissor() { return _scissor; }

        void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~ViewportState();

        VkViewport _viewport;
        VkRect2D _scissor;
    };
    VSG_type_name(vsg::ViewportState);

    class VSG_DECLSPEC RasterizationState : public Inherit<GraphicsPipelineState, RasterizationState>, public VkPipelineRasterizationStateCreateInfo
    {
    public:
        RasterizationState();

        VkStructureType getType() const override { return sType; }

        void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~RasterizationState();
    };
    VSG_type_name(vsg::RasterizationState);

    class VSG_DECLSPEC MultisampleState : public Inherit<GraphicsPipelineState, MultisampleState>, public VkPipelineMultisampleStateCreateInfo
    {
    public:
        MultisampleState();

        VkStructureType getType() const override { return sType; }

        void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~MultisampleState();
    };
    VSG_type_name(vsg::MultisampleState);

    class VSG_DECLSPEC DepthStencilState : public Inherit<GraphicsPipelineState, DepthStencilState>, public VkPipelineDepthStencilStateCreateInfo
    {
    public:
        DepthStencilState();

        void read(Input& input) override;
        void write(Output& output) const override;

        VkStructureType getType() const override { return sType; }

        void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

    protected:
        virtual ~DepthStencilState();
    };
    VSG_type_name(vsg::DepthStencilState);

    class VSG_DECLSPEC ColorBlendState : public Inherit<GraphicsPipelineState, ColorBlendState>, public VkPipelineColorBlendStateCreateInfo
    {
    public:
        using ColorBlendAttachments = std::vector<VkPipelineColorBlendAttachmentState>;

        ColorBlendState();
        ColorBlendState(const ColorBlendAttachments& colorBlendAttachments);

        void read(Input& input) override;
        void write(Output& output) const override;

        VkStructureType getType() const override { return sType; }

        void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

        const ColorBlendAttachments& getColorBlendAttachments() const { return _colorBlendAttachments; }
        void setColorBlendAttachments(const ColorBlendAttachments& colorBlendAttachments)
        {
            _colorBlendAttachments = colorBlendAttachments;
            update();
        }

        void update();

    protected:
        virtual ~ColorBlendState();

        ColorBlendAttachments _colorBlendAttachments;
    };
    VSG_type_name(vsg::ColorBlendState);

} // namespace vsg
