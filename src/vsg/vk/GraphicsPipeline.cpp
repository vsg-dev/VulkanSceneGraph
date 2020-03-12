/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/GraphicsPipeline.h>

#include <vsg/traversals/CompileTraversal.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////
//
// GraphicsPipeline
//
GraphicsPipeline::GraphicsPipeline()
{
}

GraphicsPipeline::GraphicsPipeline(PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const GraphicsPipelineStates& pipelineStates, uint32_t subpass, AllocationCallbacks* allocator) :
    _pipelineLayout(pipelineLayout),
    _shaderStages(shaderStages),
    _pipelineStates(pipelineStates),
    _subpass(subpass),
    _allocator(allocator)
{
}

GraphicsPipeline::~GraphicsPipeline()
{
}

void GraphicsPipeline::read(Input& input)
{
    Object::read(input);

    _pipelineLayout = input.readObject<PipelineLayout>("PipelineLayout");

    _shaderStages.resize(input.readValue<uint32_t>("NumShaderStages"));
    for (auto& shaderStage : _shaderStages)
    {
        shaderStage = input.readObject<ShaderStage>("ShaderStage");
    }

    _pipelineStates.resize(input.readValue<uint32_t>("NumPipelineStates"));
    for (auto& pipelineState : _pipelineStates)
    {
        pipelineState = input.readObject<GraphicsPipelineState>("PipelineState");
    }

    // input.read("subpass", _subpass); // TODO need to enable
}

void GraphicsPipeline::write(Output& output) const
{
    Object::write(output);

    output.writeObject("PipelineLayout", _pipelineLayout.get());

    output.writeValue<uint32_t>("NumShaderStages", _shaderStages.size());
    for (auto& shaderStage : _shaderStages)
    {
        output.writeObject("ShaderStage", shaderStage.get());
    }

    output.writeValue<uint32_t>("NumPipelineStates", _pipelineStates.size());
    for (auto& pipelineState : _pipelineStates)
    {
        output.writeObject("PipelineState", pipelineState.get());
    }

    // output.write("subpass", _subpass); // TODO need to enable
}

void GraphicsPipeline::compile(Context& context)
{
    if (!_implementation[context.deviceID])
    {
        _pipelineLayout->compile(context);

        for (auto& shaderStage : _shaderStages)
        {
            shaderStage->compile(context);
        }

        GraphicsPipelineStates full_pipelineStates = _pipelineStates;
        full_pipelineStates.emplace_back(context.viewport);

        _implementation[context.deviceID] = GraphicsPipeline::Implementation::create(context.device, context.renderPass, _pipelineLayout, _shaderStages, full_pipelineStates, _subpass, _allocator);
    }
}

////////////////////////////////////////////////////////////////////////
//
// GraphicsPipeline::Implementation
//
GraphicsPipeline::Implementation::Implementation(VkPipeline pipeline, Device* device, RenderPass* renderPass, PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const GraphicsPipelineStates& pipelineStates, AllocationCallbacks* allocator) :
    _pipeline(pipeline),
    _device(device),
    _renderPass(renderPass),
    _pipelineLayout(pipelineLayout),
    _shaderStages(shaderStages),
    _pipelineStates(pipelineStates),
    _allocator(allocator)
{
}

GraphicsPipeline::Implementation::Result GraphicsPipeline::Implementation::create(Device* device, RenderPass* renderPass, PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const GraphicsPipelineStates& pipelineStates, uint32_t subpass, AllocationCallbacks* allocator)
{
    if (!device || !renderPass || !pipelineLayout)
    {
        return Result("Error: vsg::GraphicsPipeline::create(...) failed to create graphics pipeline, inputs not defined.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = pipelineLayout->vk(device->deviceID);
    pipelineInfo.renderPass = *renderPass;
    pipelineInfo.subpass = subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pNext = nullptr;

    std::vector<VkSpecializationInfo> specializationInfos(shaderStages.size());
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo(shaderStages.size());
    for (size_t i = 0; i < shaderStages.size(); ++i)
    {
        const ShaderStage* shaderStage = shaderStages[i];
        shaderStageCreateInfo[i].pNext = nullptr;
        shaderStage->apply(device->deviceID, shaderStageCreateInfo[i]);
        if (!shaderStage->getSpecializationMapEntries().empty() && shaderStage->getSpecializationData() != nullptr)
        {
            // assign a VkSpecializationInfo for this shaderStageCreateInfo
            VkSpecializationInfo& specializationInfo = specializationInfos[i];
            shaderStageCreateInfo[i].pSpecializationInfo = &specializationInfo;

            // assign the values from the ShaderStage into the specializationInfo
            specializationInfo.mapEntryCount = static_cast<uint32_t>(shaderStage->getSpecializationMapEntries().size());
            specializationInfo.pMapEntries = shaderStage->getSpecializationMapEntries().data();
            specializationInfo.dataSize = shaderStage->getSpecializationData()->dataSize();
            specializationInfo.pData = shaderStage->getSpecializationData()->dataPointer();
        }
    }

    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStageCreateInfo.size());
    pipelineInfo.pStages = shaderStageCreateInfo.data();

    for (auto pipelineState : pipelineStates)
    {
        pipelineState->apply(pipelineInfo);
    }

    VkPipeline pipeline;
    VkResult result = vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, allocator, &pipeline);
    if (result == VK_SUCCESS)
    {
        return Result(new Implementation(pipeline, device, renderPass, pipelineLayout, shaderStages, pipelineStates, allocator));
    }
    else
    {
        return Result("Error: vsg::Pipeline::createGraphics(...) failed to create VkPipeline.", result);
    }
}

GraphicsPipeline::Implementation::~Implementation()
{
    vkDestroyPipeline(*_device, _pipeline, _allocator);
}

////////////////////////////////////////////////////////////////////////
//
// BindGraphicsPipeline
//
BindGraphicsPipeline::BindGraphicsPipeline(GraphicsPipeline* pipeline) :
    Inherit(0), // slot 0
    _pipeline(pipeline)
{
}

BindGraphicsPipeline::~BindGraphicsPipeline()
{
}

void BindGraphicsPipeline::read(Input& input)
{
    StateCommand::read(input);

    _pipeline = input.readObject<GraphicsPipeline>("GraphicsPipeline");
}

void BindGraphicsPipeline::write(Output& output) const
{
    StateCommand::write(output);

    output.writeObject("GraphicsPipeline", _pipeline.get());
}

void BindGraphicsPipeline::dispatch(CommandBuffer& commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->vk(commandBuffer.deviceID));
    commandBuffer.setCurrentPipelineLayout(_pipeline->getPipelineLayout()->vk(commandBuffer.deviceID));
}

void BindGraphicsPipeline::compile(Context& context)
{
    if (_pipeline) _pipeline->compile(context);
}

void BindGraphicsPipeline::release()
{
    if (_pipeline) _pipeline->release();
}

////////////////////////////////////////////////////////////////////////
//
// VertexInputState
//
VertexInputState::VertexInputState() :
    VkPipelineVertexInputStateCreateInfo{}
{
    sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexBindingDescriptionCount = 0;
    vertexAttributeDescriptionCount = 0;
    pNext = nullptr;
}

VertexInputState::VertexInputState(const Bindings& bindings, const Attributes& attributes) :
    VkPipelineVertexInputStateCreateInfo{},
    _bindings(bindings),
    _attributes(attributes)
{
    sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pNext = nullptr;
    _assign();
}

VertexInputState::~VertexInputState()
{
}

void VertexInputState::_assign()
{
    vertexBindingDescriptionCount = static_cast<uint32_t>(_bindings.size());
    pVertexBindingDescriptions = _bindings.data();
    vertexAttributeDescriptionCount = static_cast<uint32_t>(_attributes.size());
    pVertexAttributeDescriptions = _attributes.data();
}

void VertexInputState::read(Input& input)
{
    Object::read(input);

    _bindings.resize(input.readValue<uint32_t>("NumBindings"));
    for (auto& binding : _bindings)
    {
        input.read("binding", binding.binding);
        input.read("stride", binding.stride);
        binding.inputRate = static_cast<VkVertexInputRate>(input.readValue<uint32_t>("inputRate"));
    }

    _attributes.resize(input.readValue<uint32_t>("NumAttributes"));
    for (auto& attribute : _attributes)
    {
        input.read("location", attribute.location);
        input.read("binding", attribute.binding);
        attribute.format = static_cast<VkFormat>(input.readValue<uint32_t>("format"));
        input.read("offset", attribute.offset);
    }

    _assign();
}

void VertexInputState::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("NumBindings", _bindings.size());
    for (auto& binding : _bindings)
    {
        output.write("binding", binding.binding);
        output.write("stride", binding.stride);
        output.writeValue<uint32_t>("inputRate", binding.inputRate);
    }

    output.writeValue<uint32_t>("NumAttributes", _attributes.size());
    for (auto& attribute : _attributes)
    {
        output.write("location", attribute.location);
        output.write("binding", attribute.binding);
        output.writeValue<uint32_t>("format", attribute.format);
        output.write("offset", attribute.offset);
    }
}

void VertexInputState::apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    pipelineInfo.pVertexInputState = this;
}

////////////////////////////////////////////////////////////////////////
//
// InputAssemblyState
//
InputAssemblyState::InputAssemblyState() :
    VkPipelineInputAssemblyStateCreateInfo{}
{
    sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    primitiveRestartEnable = VK_FALSE;
    pNext = nullptr;
}

InputAssemblyState::InputAssemblyState(VkPrimitiveTopology primitiveTopology, bool enablePrimitiveRestart) :
    VkPipelineInputAssemblyStateCreateInfo{}
{
    sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    topology = primitiveTopology;
    primitiveRestartEnable = enablePrimitiveRestart ? VK_TRUE : VK_FALSE;
    pNext = nullptr;
}

InputAssemblyState::~InputAssemblyState()
{
}

void InputAssemblyState::read(Input& input)
{
    Object::read(input);

    topology = static_cast<VkPrimitiveTopology>(input.readValue<uint32_t>("topology"));
    primitiveRestartEnable = static_cast<VkBool32>(input.readValue<uint32_t>("primitiveRestartEnable"));
}

void InputAssemblyState::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("topology", topology);
    output.writeValue<uint32_t>("primitiveRestartEnable", primitiveRestartEnable);
}

void InputAssemblyState::apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    pipelineInfo.pInputAssemblyState = this;
}

////////////////////////////////////////////////////////////////////////
//
// ViewportState
//
ViewportState::ViewportState() :
    VkPipelineViewportStateCreateInfo{},
    _viewport{},
    _scissor{}
{
    sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pNext = nullptr;
}

ViewportState::ViewportState(const VkExtent2D& extent) :
    VkPipelineViewportStateCreateInfo{},
    _viewport{},
    _scissor{}
{
    sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pNext = nullptr;
    _viewport.x = 0.0f;
    _viewport.y = 0.0f;
    _viewport.width = static_cast<float>(extent.width);
    _viewport.height = static_cast<float>(extent.height);
    _viewport.minDepth = 0.0f;
    _viewport.maxDepth = 1.0f;

    _scissor.offset = {0, 0};
    _scissor.extent = extent;

    viewportCount = 1;
    pViewports = &_viewport;
    scissorCount = 1;
    pScissors = &_scissor;
}

ViewportState::~ViewportState()
{
}

void ViewportState::apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    pipelineInfo.pViewportState = this;
}

////////////////////////////////////////////////////////////////////////
//
// RasterizationState
//
RasterizationState::RasterizationState() :
    VkPipelineRasterizationStateCreateInfo{}
{
    sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    depthClampEnable = VK_FALSE;
    polygonMode = VK_POLYGON_MODE_FILL;
    lineWidth = 1.0f;
    cullMode = VK_CULL_MODE_BACK_BIT;
    //    frontFace = VK_FRONT_FACE_CLOCKWISE;
    frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    depthBiasEnable = VK_FALSE;
    pNext = nullptr;
}

RasterizationState::~RasterizationState()
{
}

void RasterizationState::apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    pipelineInfo.pRasterizationState = this;
}

////////////////////////////////////////////////////////////////////////
//
// MultisampleState
//
MultisampleState::MultisampleState() :
    VkPipelineMultisampleStateCreateInfo{}
{
    sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    sampleShadingEnable = VK_FALSE;
    rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pNext = nullptr;
}

MultisampleState::~MultisampleState()
{
}

void MultisampleState::apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    pipelineInfo.pMultisampleState = this;
}

////////////////////////////////////////////////////////////////////////
//
// DepthStencilState
//
DepthStencilState::DepthStencilState() :
    VkPipelineDepthStencilStateCreateInfo{}
{
    sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthTestEnable = VK_TRUE;
    depthWriteEnable = VK_TRUE;
    depthCompareOp = VK_COMPARE_OP_LESS;
    depthBoundsTestEnable = VK_FALSE;
    stencilTestEnable = VK_FALSE;
    pNext = nullptr;
}

DepthStencilState::~DepthStencilState()
{
}

void DepthStencilState::read(Input& input)
{
    Object::read(input);

    depthTestEnable = static_cast<VkBool32>(input.readValue<uint32_t>("depthTestEnable"));
    depthWriteEnable = static_cast<VkBool32>(input.readValue<uint32_t>("depthWriteEnable"));
    depthCompareOp = static_cast<VkCompareOp>(input.readValue<uint32_t>("depthCompareOp"));
    depthBoundsTestEnable = static_cast<VkBool32>(input.readValue<uint32_t>("depthBoundsTestEnable"));
    stencilTestEnable = static_cast<VkBool32>(input.readValue<uint32_t>("stencilTestEnable"));
}

void DepthStencilState::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("depthTestEnable", depthTestEnable);
    output.writeValue<uint32_t>("depthWriteEnable", depthWriteEnable);
    output.writeValue<uint32_t>("depthCompareOp", depthCompareOp);
    output.writeValue<uint32_t>("depthBoundsTestEnable", depthBoundsTestEnable);
    output.writeValue<uint32_t>("stencilTestEnable", stencilTestEnable);
}

void DepthStencilState::apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    pipelineInfo.pDepthStencilState = this;
}

////////////////////////////////////////////////////////////////////////
//
// ColorBlendState
//
ColorBlendState::ColorBlendState() :
    VkPipelineColorBlendStateCreateInfo{}
{
    sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT;

    _colorBlendAttachments.push_back(colorBlendAttachment);

    logicOpEnable = VK_FALSE;
    logicOp = VK_LOGIC_OP_COPY;
    attachmentCount = static_cast<uint32_t>(_colorBlendAttachments.size());
    pAttachments = _colorBlendAttachments.data();
    blendConstants[0] = 0.0f;
    blendConstants[1] = 0.0f;
    blendConstants[2] = 0.0f;
    blendConstants[3] = 0.0f;
    pNext = nullptr;
}

ColorBlendState::ColorBlendState(const ColorBlendAttachments& colorBlendAttachments) :
    ColorBlendState()
{
    setColorBlendAttachments(colorBlendAttachments);
}

ColorBlendState::~ColorBlendState()
{
}

void ColorBlendState::read(Input& input)
{
    Object::read(input);

    logicOp = static_cast<VkLogicOp>(input.readValue<uint32_t>("logicOp"));
    logicOpEnable = static_cast<VkBool32>(input.readValue<uint32_t>("logicOpEnable"));

    _colorBlendAttachments.resize(input.readValue<uint32_t>("NumColorBlendAttachments"));
    for (auto& colorBlendAttachment : _colorBlendAttachments)
    {
        colorBlendAttachment.blendEnable = static_cast<VkBool32>(input.readValue<uint32_t>("blendEnable"));
        colorBlendAttachment.srcColorBlendFactor = static_cast<VkBlendFactor>(input.readValue<uint32_t>("srcColorBlendFactor"));
        colorBlendAttachment.dstColorBlendFactor = static_cast<VkBlendFactor>(input.readValue<uint32_t>("dstColorBlendFactor"));
        colorBlendAttachment.colorBlendOp = static_cast<VkBlendOp>(input.readValue<uint32_t>("colorBlendOp"));
        colorBlendAttachment.srcAlphaBlendFactor = static_cast<VkBlendFactor>(input.readValue<uint32_t>("srcAlphaBlendFactor"));
        colorBlendAttachment.dstAlphaBlendFactor = static_cast<VkBlendFactor>(input.readValue<uint32_t>("dstAlphaBlendFactor"));
        colorBlendAttachment.alphaBlendOp = static_cast<VkBlendOp>(input.readValue<uint32_t>("alphaBlendOp"));
        colorBlendAttachment.colorWriteMask = static_cast<VkColorComponentFlags>(input.readValue<uint32_t>("colorWriteMask"));
    }

    input.read("blendConstants0", blendConstants[0]);
    input.read("blendConstants1", blendConstants[1]);
    input.read("blendConstants2", blendConstants[2]);
    input.read("blendConstants3", blendConstants[3]);

    update();
}

void ColorBlendState::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("logicOp", logicOp);
    output.writeValue<uint32_t>("logicOpEnable", logicOpEnable);

    output.writeValue<uint32_t>("NumColorBlendAttachments", _colorBlendAttachments.size());
    for (auto& colorBlendAttachment : _colorBlendAttachments)
    {
        output.writeValue<uint32_t>("blendEnable", colorBlendAttachment.blendEnable);
        output.writeValue<uint32_t>("srcColorBlendFactor", colorBlendAttachment.srcColorBlendFactor);
        output.writeValue<uint32_t>("dstColorBlendFactor", colorBlendAttachment.dstColorBlendFactor);
        output.writeValue<uint32_t>("colorBlendOp", colorBlendAttachment.colorBlendOp);
        output.writeValue<uint32_t>("srcAlphaBlendFactor", colorBlendAttachment.srcAlphaBlendFactor);
        output.writeValue<uint32_t>("dstAlphaBlendFactor", colorBlendAttachment.dstAlphaBlendFactor);
        output.writeValue<uint32_t>("alphaBlendOp", colorBlendAttachment.alphaBlendOp);
        output.writeValue<uint32_t>("colorWriteMask", colorBlendAttachment.colorWriteMask);
    }

    output.write("blendConstants0", blendConstants[0]);
    output.write("blendConstants1", blendConstants[1]);
    output.write("blendConstants2", blendConstants[2]);
    output.write("blendConstants3", blendConstants[3]);
}

void ColorBlendState::apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    pipelineInfo.pColorBlendState = this;
}

void ColorBlendState::update()
{
    attachmentCount = static_cast<uint32_t>(_colorBlendAttachments.size());
    pAttachments = _colorBlendAttachments.data();
}
