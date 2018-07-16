#pragma once

#include <vsg/vk/Swapchain.h>
#include <vsg/vk/ShaderModule.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/PipelineLayout.h>
#include <vsg/vk/CmdDraw.h>

namespace vsg
{

    class Pipeline : public Dispatch
    {
    public:
        Pipeline(Device* device, VkPipeline pipeline, AllocationCallbacks* allocator=nullptr);

        operator VkPipeline () const { return _pipeline; }

        virtual void dispatch(VkCommandBuffer commandBuffer) const
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
        }

    protected:
        virtual ~Pipeline();

        ref_ptr<Device>                 _device;
        VkPipeline                      _pipeline;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

    // should this be called GraphicsPipelineState?
    class PipelineState : public Object
    {
    public:
        PipelineState() {}

        virtual VkStructureType getType() const = 0;

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const = 0;

    protected:
        virtual ~PipelineState() {}
    };

    class ShaderStages : public PipelineState
    {
    public:
        ShaderStages(const ShaderModules& shaderModules)
        {
            setShaderModules(shaderModules);
        }

        virtual VkStructureType getType() const { return VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; }

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
        {
            pipelineInfo.stageCount = size();
            pipelineInfo.pStages = data();
        }

        void setShaderModules(const ShaderModules& shaderModules) { _shaderModules = shaderModules; update(); }
        const ShaderModules& getShaderModules() const { return _shaderModules; }

        void update()
        {
            _stages.resize(_shaderModules.size());
            for (size_t i=0; i<_shaderModules.size(); ++i)
            {
                VkPipelineShaderStageCreateInfo& stageInfo = (_stages)[i];
                ShaderModule* sm = _shaderModules[i];
                stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                stageInfo.stage = sm->getStage();
                stageInfo.module = *sm;
                stageInfo.pName = sm->getEntryPointName().c_str();
            }
        }

        std::size_t size() const { return _stages.size(); }

        VkPipelineShaderStageCreateInfo* data() { return _stages.data(); }
        const VkPipelineShaderStageCreateInfo* data() const { return _stages.data(); }

    protected:
        virtual ~ShaderStages()
        {
            std::cout<<"~ShaderStages()"<<std::endl;
        }

        using Stages = std::vector<VkPipelineShaderStageCreateInfo>;
        Stages          _stages;
        ShaderModules   _shaderModules;
    };

    class VertexInputState : public PipelineState, public VkPipelineVertexInputStateCreateInfo
    {
    public:
        using Bindings = std::vector<VkVertexInputBindingDescription>;
        using Attributes = std::vector<VkVertexInputAttributeDescription>;

        VertexInputState() :
            VkPipelineVertexInputStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO}
        {
            vertexBindingDescriptionCount = 0;
            vertexAttributeDescriptionCount = 0;
        }

        VertexInputState(const Bindings& bindings, const Attributes& attributes) :
            _bindings(bindings),
            _attributes(attributes),
            VkPipelineVertexInputStateCreateInfo{}
        {
            sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexBindingDescriptionCount = _bindings.size();
            pVertexBindingDescriptions = _bindings.data();
            vertexAttributeDescriptionCount = _attributes.size();
            pVertexAttributeDescriptions = _attributes.data();
        }

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
        {
            pipelineInfo.pVertexInputState = this;
        }

        virtual VkStructureType getType() const { return sType; }

        const Bindings& geBindings() { return _bindings; }

        const Attributes& getAttributes() const { return _attributes; }

    protected:
        virtual ~VertexInputState()
        {
            std::cout<<"~VertexInputState()"<<std::endl;
        }

        Bindings                                _bindings;
        Attributes                              _attributes;
    };

    class InputAssemblyState : public PipelineState, public VkPipelineInputAssemblyStateCreateInfo
    {
    public:
        InputAssemblyState() :
            VkPipelineInputAssemblyStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO}
        {
            // primitive input
            topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            primitiveRestartEnable = VK_FALSE;
        }

        virtual VkStructureType getType() const { return sType; }

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
        {
            pipelineInfo.pInputAssemblyState = this;
        }

    protected:
        virtual ~InputAssemblyState()
        {
            std::cout<<"~InputAssemblyState()"<<std::endl;
        }
    };


    class ViewportState : public PipelineState, public VkPipelineViewportStateCreateInfo
    {
    public:
        ViewportState(const VkExtent2D& extent) :
            VkPipelineViewportStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO},
            _viewport{},
            _scissor{}
        {
            _viewport.x = 0.0f;
            _viewport.y = 0.0f;
            _viewport.width = static_cast<float>(extent.width);
            _viewport.height = static_cast<float>(extent.height);

            _scissor.offset = {0, 0};
            _scissor.extent = extent;

            viewportCount = 1;
            pViewports = &_viewport;
            scissorCount = 1;
            pScissors = &_scissor;
        }

        virtual VkStructureType getType() const { return sType; }

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
        {
            pipelineInfo.pViewportState = this;
        }

        VkViewport& getViewport() { return _viewport; }
        VkRect2D& getScissor() { return _scissor; }

    protected:
        virtual ~ViewportState()
        {
            std::cout<<"~ViewportState()"<<std::endl;
        }

        VkViewport                          _viewport;
        VkRect2D                            _scissor;
    };

    class RasterizationState : public PipelineState, public VkPipelineRasterizationStateCreateInfo
    {
    public:
        RasterizationState() :
            VkPipelineRasterizationStateCreateInfo {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO}
        {
            depthClampEnable = VK_FALSE;
            polygonMode = VK_POLYGON_MODE_FILL;
            lineWidth = 1.0f;
            cullMode = VK_CULL_MODE_BACK_BIT;
            frontFace = VK_FRONT_FACE_CLOCKWISE;
            depthBiasEnable = VK_FALSE;
        }

        virtual VkStructureType getType() const { return sType; }

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
        {
            pipelineInfo.pRasterizationState = this;
        }

    protected:
        virtual ~RasterizationState()
        {
            std::cout<<"~RasterizationState()"<<std::endl;
        }
    };

    class MultisampleState : public PipelineState, public VkPipelineMultisampleStateCreateInfo
    {
    public:
        MultisampleState() :
            VkPipelineMultisampleStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO}
        {
            sampleShadingEnable =VK_FALSE;
            rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        }

        virtual VkStructureType getType() const { return sType; }

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
        {
            pipelineInfo.pMultisampleState = this;
        }

    protected:
        virtual ~MultisampleState()
        {
            std::cout<<"~MultisampleState()"<<std::endl;
        }

        VkPipelineMultisampleStateCreateInfo _info;
    };


    class ColorBlendState : public PipelineState, public VkPipelineColorBlendStateCreateInfo
    {
    public:
        ColorBlendState() :
            VkPipelineColorBlendStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO}
        {
            VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
            colorBlendAttachment.blendEnable = VK_FALSE;
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                VK_COLOR_COMPONENT_G_BIT |
                                                VK_COLOR_COMPONENT_B_BIT |
                                                VK_COLOR_COMPONENT_A_BIT;

            _colorBlendAttachments.push_back(colorBlendAttachment);

            logicOpEnable = VK_FALSE;
            logicOp = VK_LOGIC_OP_COPY;
            attachmentCount = _colorBlendAttachments.size();
            pAttachments = _colorBlendAttachments.data();
            blendConstants[0] = 0.0f;
            blendConstants[1] = 0.0f;
            blendConstants[2] = 0.0f;
            blendConstants[3] = 0.0f;
        }

        virtual VkStructureType getType() const { return sType; }

        virtual void apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
        {
            pipelineInfo.pColorBlendState = this;
        }

        using ColorBlendAttachments = std::vector<VkPipelineColorBlendAttachmentState>;
        const ColorBlendAttachments& getColorBlendAttachments() const { return _colorBlendAttachments; }

        void update()
        {
            attachmentCount = _colorBlendAttachments.size();
            pAttachments = _colorBlendAttachments.data();
        }

    protected:
        virtual ~ColorBlendState()
        {
            std::cout<<"~ColorBlendState()"<<std::endl;
        }

        ColorBlendAttachments _colorBlendAttachments;
    };

    using PipelineStates = std::vector<ref_ptr<PipelineState>>;
    using PipelineResult = vsg::Result<Pipeline, VkResult, VK_SUCCESS>;

    PipelineResult createGraphics(Device* device, RenderPass* renderPass, PipelineLayout* pipelineLayout, const PipelineStates& pipelineStates, AllocationCallbacks* allocator=nullptr)
    {
        if (!device || !renderPass || !pipelineLayout)
        {
            return PipelineResult("Error: vsg::createGraphics(...) failed to create graphics pipeline, undefined inputs.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
        }

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = *pipelineLayout;
        pipelineInfo.renderPass = *renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        for (auto pipelineState : pipelineStates)
        {
            pipelineState->apply(pipelineInfo);
        }

        VkPipeline pipeline;
        VkResult result = vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, *allocator, &pipeline );
        if (result == VK_SUCCESS)
        {
            return new Pipeline(device, pipeline, allocator);
        }
        else
        {
            return PipelineResult("Error: vsg::createGraphics(...) failed to create VkPipeline.", result);
        }
    }

}
