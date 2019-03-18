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
#include <vsg/vk/ShaderModule.h>

namespace vsg
{

    class GraphicsPipelineState : public Inherit<Object, GraphicsPipelineState>
    {
    public:
        GraphicsPipelineState() {}

        virtual VkStructureType getType() const = 0;

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const = 0;

        // compile the Vulkan object, context parameter used for Device
        virtual void compile(Context& /*context*/) {}

        // remove the local reference to the Vulkan implementation
        virtual void release() {}

    protected:
        virtual ~GraphicsPipelineState() {}
    };
    VSG_type_name(vsg::GraphicsPipelineState);

    using GraphicsPipelineStates = std::vector<ref_ptr<GraphicsPipelineState>>;

    class VSG_DECLSPEC GraphicsPipeline : public Inherit<Object, GraphicsPipeline>
    {
    public:
        GraphicsPipeline();

        GraphicsPipeline(PipelineLayout* pipelineLayout, const GraphicsPipelineStates& pipelineStates, AllocationCallbacks* allocator = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        PipelineLayout* getPipelineLayout() { return _pipelineLayout; }
        const PipelineLayout* getPipelineLayout() const { return _pipelineLayout; }

        GraphicsPipelineStates& getPipelineStates() { return _pipelineStates; }
        const GraphicsPipelineStates& getPipelineStates() const { return _pipelineStates; }

        class VSG_DECLSPEC Implementation : public Inherit<Object, Implementation>
        {
        public:
            Implementation(VkPipeline pipeline, Device* device, RenderPass* renderPass, PipelineLayout* pipelineLayout, const GraphicsPipelineStates& pipelineStates, AllocationCallbacks* allocator = nullptr);
            virtual ~Implementation();

            using Result = vsg::Result<Implementation, VkResult, VK_SUCCESS>;

            /** Crreate a GraphicsPipeline.*/
            static Result create(Device* device, RenderPass* renderPass, PipelineLayout* pipelineLayout, const GraphicsPipelineStates& pipelineStates, AllocationCallbacks* allocator = nullptr);

            VkPipeline _pipeline;

            // TODO need to convert to use Implementation versions of RenderPass and PipelineLayout
            ref_ptr<Device> _device;
            ref_ptr<RenderPass> _renderPass;
            ref_ptr<PipelineLayout> _pipelineLayout;
            GraphicsPipelineStates _pipelineStates;
            ref_ptr<AllocationCallbacks> _allocator;
        };

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release() { _implementation = nullptr; }

        operator VkPipeline() const { return _implementation->_pipeline; }

    protected:
        virtual ~GraphicsPipeline();

        ref_ptr<Device> _device;
        ref_ptr<RenderPass> _renderPass;
        ref_ptr<PipelineLayout> _pipelineLayout;
        GraphicsPipelineStates _pipelineStates;
        ref_ptr<AllocationCallbacks> _allocator;

        ref_ptr<Implementation> _implementation;
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

        void pushTo(State& state) const override;
        void popFrom(State& state) const override;
        void dispatch(CommandBuffer& commandBuffer) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context) override;

    public:
        virtual ~BindGraphicsPipeline();

        ref_ptr<GraphicsPipeline> _pipeline;
    };
    VSG_type_name(vsg::BindGraphicsPipeline);

    class VSG_DECLSPEC ShaderStages : public Inherit<GraphicsPipelineState, ShaderStages>
    {
    public:
        ShaderStages();
        ShaderStages(const ShaderModules& shaderModules);

        void read(Input& input) override;
        void write(Output& output) const override;

        VkStructureType getType() const override { return VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; }

        void setShaderModules(const ShaderModules& shaderModules)
        {
            _shaderModules = shaderModules;
        }
        const ShaderModules& getShaderModules() const { return _shaderModules; }

        std::size_t size() const { return _stages.size(); }

        VkPipelineShaderStageCreateInfo* data() { return _stages.data(); }
        const VkPipelineShaderStageCreateInfo* data() const { return _stages.data(); }

        void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release();

    protected:
        virtual ~ShaderStages();

        ShaderModules _shaderModules;

        using Stages = std::vector<VkPipelineShaderStageCreateInfo>;
        Stages _stages;
    };
    VSG_type_name(vsg::ShaderStages);

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
