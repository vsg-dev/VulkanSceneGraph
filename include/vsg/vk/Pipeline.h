#pragma once

#include <vsg/vk/Swapchain.h>
#include <vsg/vk/ShaderModule.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/PipelineLayout.h>
#include <vsg/vk/CmdDraw.h>

namespace vsg
{

    class Pipeline : public Command
    {
    public:
        Pipeline(Device* device, VkPipeline pipeline, VkPipelineBindPoint bindPoint, AllocationCallbacks* allocator=nullptr);

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        operator VkPipeline () const { return _pipeline; }

        virtual void dispatch(VkCommandBuffer commandBuffer) const
        {
            vkCmdBindPipeline(commandBuffer, _bindPoint, _pipeline);
        }

    protected:
        virtual ~Pipeline();

        ref_ptr<Device>                 _device;
        VkPipeline                      _pipeline;
        VkPipelineBindPoint             _bindPoint;
        ref_ptr<AllocationCallbacks>    _allocator;
    };


    class GraphicsPipelineState : public Object
    {
    public:
        GraphicsPipelineState() {}

        virtual VkStructureType getType() const = 0;

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const = 0;

    protected:
        virtual ~GraphicsPipelineState() {}
    };


    class ShaderStages : public GraphicsPipelineState
    {
    public:
        ShaderStages(const ShaderModules& shaderModules);

        virtual VkStructureType getType() const { return VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; }

        void setShaderModules(const ShaderModules& shaderModules) { _shaderModules = shaderModules; update(); }
        const ShaderModules& getShaderModules() const { return _shaderModules; }

        void update();

        std::size_t size() const { return _stages.size(); }

        VkPipelineShaderStageCreateInfo* data() { return _stages.data(); }
        const VkPipelineShaderStageCreateInfo* data() const { return _stages.data(); }

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const;

    protected:
        virtual ~ShaderStages();

        using Stages = std::vector<VkPipelineShaderStageCreateInfo>;
        Stages          _stages;
        ShaderModules   _shaderModules;
    };


    class VertexInputState : public GraphicsPipelineState, public VkPipelineVertexInputStateCreateInfo
    {
    public:
        using Bindings = std::vector<VkVertexInputBindingDescription>;
        using Attributes = std::vector<VkVertexInputAttributeDescription>;

        VertexInputState();
        VertexInputState(const Bindings& bindings, const Attributes& attributes);

        virtual VkStructureType getType() const { return sType; }

        const Bindings& geBindings() { return _bindings; }

        const Attributes& getAttributes() const { return _attributes; }

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const;

    protected:
        virtual ~VertexInputState();

        Bindings                                _bindings;
        Attributes                              _attributes;
    };

    class InputAssemblyState : public GraphicsPipelineState, public VkPipelineInputAssemblyStateCreateInfo
    {
    public:
        InputAssemblyState();

        virtual VkStructureType getType() const { return sType; }

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const;

    protected:
        virtual ~InputAssemblyState();
    };


    class ViewportState : public GraphicsPipelineState, public VkPipelineViewportStateCreateInfo
    {
    public:
        ViewportState(const VkExtent2D& extent);

        virtual VkStructureType getType() const { return sType; }

        VkViewport& getViewport() { return _viewport; }
        VkRect2D& getScissor() { return _scissor; }

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const;

    protected:
        virtual ~ViewportState();

        VkViewport                          _viewport;
        VkRect2D                            _scissor;
    };

    class RasterizationState : public GraphicsPipelineState, public VkPipelineRasterizationStateCreateInfo
    {
    public:
        RasterizationState();

        virtual VkStructureType getType() const { return sType; }

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const;

    protected:
        virtual ~RasterizationState();
    };

    class MultisampleState : public GraphicsPipelineState, public VkPipelineMultisampleStateCreateInfo
    {
    public:
        MultisampleState();

        virtual VkStructureType getType() const { return sType; }

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const;

    protected:
        virtual ~MultisampleState();

        VkPipelineMultisampleStateCreateInfo _info;
    };


    class ColorBlendState : public GraphicsPipelineState, public VkPipelineColorBlendStateCreateInfo
    {
    public:
        ColorBlendState();

        virtual VkStructureType getType() const { return sType; }

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const;

        using ColorBlendAttachments = std::vector<VkPipelineColorBlendAttachmentState>;
        const ColorBlendAttachments& getColorBlendAttachments() const { return _colorBlendAttachments; }

        void update();

    protected:
        virtual ~ColorBlendState();

        ColorBlendAttachments _colorBlendAttachments;
    };

    using GraphicsPipelineStates = std::vector<ref_ptr<GraphicsPipelineState>>;
    using PipelineResult = vsg::Result<Pipeline, VkResult, VK_SUCCESS>;

    extern PipelineResult createGraphicsPipeline(Device* device, RenderPass* renderPass, PipelineLayout* pipelineLayout, const GraphicsPipelineStates& pipelineStates, AllocationCallbacks* allocator=nullptr);

}
