/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/state/GraphicsPipelineStates.h>
#include <vsg/vk/Context.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////
//
// VertexInputState
//
VertexInputState::VertexInputState()
{
}

VertexInputState::VertexInputState(const Bindings& bindings, const Attributes& attributes) :
    vertexBindingDescriptions(bindings),
    vertexAttributeDescriptions(attributes)
{
}

VertexInputState::~VertexInputState()
{
}

void VertexInputState::read(Input& input)
{
    Object::read(input);

    vertexBindingDescriptions.resize(input.readValue<uint32_t>("NumBindings"));
    for (auto& binding : vertexBindingDescriptions)
    {
        input.read("binding", binding.binding);
        input.read("stride", binding.stride);
        binding.inputRate = static_cast<VkVertexInputRate>(input.readValue<uint32_t>("inputRate"));
    }

    vertexAttributeDescriptions.resize(input.readValue<uint32_t>("NumAttributes"));
    for (auto& attribute : vertexAttributeDescriptions)
    {
        input.read("location", attribute.location);
        input.read("binding", attribute.binding);
        attribute.format = static_cast<VkFormat>(input.readValue<uint32_t>("format"));
        input.read("offset", attribute.offset);
    }
}

void VertexInputState::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("NumBindings", vertexBindingDescriptions.size());
    for (auto& binding : vertexBindingDescriptions)
    {
        output.write("binding", binding.binding);
        output.write("stride", binding.stride);
        output.writeValue<uint32_t>("inputRate", binding.inputRate);
    }

    output.writeValue<uint32_t>("NumAttributes", vertexAttributeDescriptions.size());
    for (auto& attribute : vertexAttributeDescriptions)
    {
        output.write("location", attribute.location);
        output.write("binding", attribute.binding);
        output.writeValue<uint32_t>("format", attribute.format);
        output.write("offset", attribute.offset);
    }
}

void VertexInputState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto vertexInputState = context.scratchMemory->allocate<VkPipelineVertexInputStateCreateInfo>();

    vertexInputState->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState->pNext = nullptr;
    vertexInputState->flags = 0;
    vertexInputState->vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size());
    vertexInputState->pVertexBindingDescriptions = vertexBindingDescriptions.data();
    vertexInputState->vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
    vertexInputState->pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

    pipelineInfo.pVertexInputState = vertexInputState;
}

////////////////////////////////////////////////////////////////////////
//
// InputAssemblyState
//
InputAssemblyState::InputAssemblyState()
{
}

InputAssemblyState::InputAssemblyState(VkPrimitiveTopology primitiveTopology, VkBool32 primitiveRestart) :
    topology(primitiveTopology),
    primitiveRestartEnable(primitiveRestart)
{
}

InputAssemblyState::~InputAssemblyState()
{
}

void InputAssemblyState::read(Input& input)
{
    Object::read(input);

    topology = static_cast<VkPrimitiveTopology>(input.readValue<uint32_t>("topology"));
    primitiveRestartEnable = input.readValue<uint32_t>("primitiveRestartEnable") != 0;
}

void InputAssemblyState::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("topology", topology);
    output.writeValue<uint32_t>("primitiveRestartEnable", primitiveRestartEnable ? 1 : 0);
}

void InputAssemblyState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto inputAssemblyState = context.scratchMemory->allocate<VkPipelineInputAssemblyStateCreateInfo>();

    inputAssemblyState->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState->pNext = nullptr;
    inputAssemblyState->flags = 0;
    inputAssemblyState->topology = topology;
    inputAssemblyState->primitiveRestartEnable = primitiveRestartEnable;

    pipelineInfo.pInputAssemblyState = inputAssemblyState;
}

////////////////////////////////////////////////////////////////////////
//
// TessellationState
//
TessellationState::TessellationState(uint32_t in_patchControlPoints) :
    patchControlPoints(in_patchControlPoints)
{
}

TessellationState::~TessellationState()
{
}

void TessellationState::read(Input& input)
{
    Object::read(input);

    input.read("patchControlPoints", patchControlPoints);
}

void TessellationState::write(Output& output) const
{
    Object::write(output);

    output.write("patchControlPoints", patchControlPoints);
}

void TessellationState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{

    auto tessellationState = context.scratchMemory->allocate<VkPipelineTessellationStateCreateInfo>();

    tessellationState->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellationState->pNext = nullptr;
    tessellationState->flags = 0;
    tessellationState->patchControlPoints = patchControlPoints;

    pipelineInfo.pTessellationState = tessellationState;
}

////////////////////////////////////////////////////////////////////////
//
// ViewportState
//
ViewportState::ViewportState()
{
}

ViewportState::ViewportState(const VkExtent2D& extent)
{
    set(extent);
}

ViewportState::~ViewportState()
{
}

void ViewportState::set(const VkExtent2D& extent)
{
    viewports.resize(1);
    scissors.resize(1);

    VkViewport& viewport = viewports[0];
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D& scissor = scissors[0];
    scissor.offset = {0, 0};
    scissor.extent = extent;
}

void ViewportState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto viewportState = context.scratchMemory->allocate<VkPipelineViewportStateCreateInfo>();

    viewportState->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState->pNext = nullptr;
    viewportState->flags = 0;

    viewportState->viewportCount = viewports.size();
    viewportState->pViewports = viewports.data();
    viewportState->scissorCount = scissors.size();
    viewportState->pScissors = scissors.data();

    pipelineInfo.pViewportState = viewportState;
}

VkViewport& ViewportState::getViewport()
{
    if (viewports.empty())
    {
        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = 0.0f;
        viewport.height = 0.0f;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        viewports.push_back(viewport);
    }
    return viewports[0];
}

VkRect2D& ViewportState::getScissor()
{
    if (scissors.empty())
    {
        VkRect2D scissor;
        scissor.offset = {0, 0};
        scissor.extent = {0, 0};
        scissors.push_back(scissor);
    }
    return scissors[0];
}

////////////////////////////////////////////////////////////////////////
//
// RasterizationState
//
RasterizationState::RasterizationState()
{
}

RasterizationState::~RasterizationState()
{
}

void RasterizationState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto rasteratizationState = context.scratchMemory->allocate<VkPipelineRasterizationStateCreateInfo>();

    rasteratizationState->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasteratizationState->pNext = nullptr;
    rasteratizationState->flags = 0;

    rasteratizationState->depthClampEnable = depthClampEnable;
    rasteratizationState->rasterizerDiscardEnable = rasterizerDiscardEnable;
    rasteratizationState->polygonMode = polygonMode;
    rasteratizationState->cullMode = cullMode;
    rasteratizationState->frontFace = frontFace;
    rasteratizationState->depthBiasEnable = depthBiasEnable;
    rasteratizationState->depthBiasConstantFactor = depthBiasConstantFactor;
    rasteratizationState->depthBiasClamp = depthBiasClamp;
    rasteratizationState->depthBiasSlopeFactor = depthBiasSlopeFactor;
    rasteratizationState->lineWidth = lineWidth;

    pipelineInfo.pRasterizationState = rasteratizationState;
}

////////////////////////////////////////////////////////////////////////
//
// MultisampleState
//
MultisampleState::MultisampleState(VkSampleCountFlagBits samples)
{
    rasterizationSamples = samples;
}

MultisampleState::~MultisampleState()
{
}

void MultisampleState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto multisampleState = context.scratchMemory->allocate<VkPipelineMultisampleStateCreateInfo>();

    multisampleState->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState->pNext = nullptr;
    multisampleState->flags = 0;

    multisampleState->rasterizationSamples = rasterizationSamples;
    multisampleState->sampleShadingEnable = sampleShadingEnable;
    multisampleState->minSampleShading = minSampleShading;
    multisampleState->pSampleMask = sampleMasks.empty() ? nullptr : sampleMasks.data();
    multisampleState->alphaToCoverageEnable = alphaToCoverageEnable;
    multisampleState->alphaToOneEnable = alphaToOneEnable;

    pipelineInfo.pMultisampleState = multisampleState;
}

////////////////////////////////////////////////////////////////////////
//
// DepthStencilState
//
DepthStencilState::DepthStencilState()
{
}

DepthStencilState::~DepthStencilState()
{
}

void DepthStencilState::read(Input& input)
{
    Object::read(input);

    input.readValue<uint32_t>("depthTestEnable", depthTestEnable);
    input.readValue<uint32_t>("depthWriteEnable", depthWriteEnable);
    input.readValue<uint32_t>("depthCompareOp", depthCompareOp);
    input.readValue<uint32_t>("depthBoundsTestEnable", depthBoundsTestEnable);
    input.readValue<uint32_t>("stencilTestEnable", stencilTestEnable);

    if (input.version_greater_equal(0, 0, 2))
    {
        input.readValue<uint32_t>("front.failOp", front.failOp);
        input.readValue<uint32_t>("front.passOp", front.passOp);
        input.readValue<uint32_t>("front.depthFailOp", front.depthFailOp);
        input.readValue<uint32_t>("front.compareOp", front.compareOp);
        input.read("front.compareMask", front.compareMask);
        input.read("front.writeMask", front.writeMask);
        input.read("front.reference", front.reference);

        input.readValue<uint32_t>("back.failOp", back.failOp);
        input.readValue<uint32_t>("back.passOp", back.passOp);
        input.readValue<uint32_t>("back.depthFailOp", back.depthFailOp);
        input.readValue<uint32_t>("back.compareOp", back.compareOp);
        input.read("back.compareMask", back.compareMask);
        input.read("back.writeMask", back.writeMask);
        input.read("back.reference", back.reference);

        input.read("minDepthBounds", minDepthBounds);
        input.read("maxDepthBounds", maxDepthBounds);
    }
}

void DepthStencilState::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("depthTestEnable", depthTestEnable);
    output.writeValue<uint32_t>("depthWriteEnable", depthWriteEnable);
    output.writeValue<uint32_t>("depthCompareOp", depthCompareOp);
    output.writeValue<uint32_t>("depthBoundsTestEnable", depthBoundsTestEnable);
    output.writeValue<uint32_t>("stencilTestEnable", stencilTestEnable);

    if (output.version_greater_equal(0, 0, 2))
    {
        output.writeValue<uint32_t>("front.failOp", front.failOp);
        output.writeValue<uint32_t>("front.passOp", front.passOp);
        output.writeValue<uint32_t>("front.depthFailOp", front.depthFailOp);
        output.writeValue<uint32_t>("front.compareOp", front.compareOp);
        output.write("front.compareMask", front.compareMask);
        output.write("front.writeMask", front.writeMask);
        output.write("front.reference", front.reference);

        output.writeValue<uint32_t>("back.failOp", back.failOp);
        output.writeValue<uint32_t>("back.passOp", back.passOp);
        output.writeValue<uint32_t>("back.depthFailOp", back.depthFailOp);
        output.writeValue<uint32_t>("back.compareOp", back.compareOp);
        output.write("back.compareMask", back.compareMask);
        output.write("back.writeMask", back.writeMask);
        output.write("back.reference", back.reference);

        output.write("minDepthBounds", minDepthBounds);
        output.write("maxDepthBounds", maxDepthBounds);
    }
}

void DepthStencilState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto depthStencilState = context.scratchMemory->allocate<VkPipelineDepthStencilStateCreateInfo>();

    depthStencilState->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState->pNext = nullptr;
    depthStencilState->flags = 0;

    depthStencilState->depthTestEnable = depthTestEnable;
    depthStencilState->depthWriteEnable = depthWriteEnable;
    depthStencilState->depthCompareOp = depthCompareOp;
    depthStencilState->depthBoundsTestEnable = depthBoundsTestEnable;
    depthStencilState->stencilTestEnable = stencilTestEnable;
    depthStencilState->front = front;
    depthStencilState->back = back;
    depthStencilState->minDepthBounds = minDepthBounds;
    depthStencilState->maxDepthBounds = maxDepthBounds;

    pipelineInfo.pDepthStencilState = depthStencilState;
}

////////////////////////////////////////////////////////////////////////
//
// ColorBlendState
//
ColorBlendState::ColorBlendState()
{
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        VK_FALSE,                                                                                                 // blendEnable
        VK_BLEND_FACTOR_ZERO,                                                                                     // srcColorBlendFactor
        VK_BLEND_FACTOR_ZERO,                                                                                     // dstColorBlendFactor
        VK_BLEND_OP_ADD,                                                                                          // colorBlendOp
        VK_BLEND_FACTOR_ZERO,                                                                                     // srcAlphaBlendFactor
        VK_BLEND_FACTOR_ZERO,                                                                                     // dstAlphaBlendFactor
        VK_BLEND_OP_ADD,                                                                                          // alphaBlendOp
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT // colorWriteMask
    };
    attachments.push_back(colorBlendAttachment);
}

ColorBlendState::ColorBlendState(const ColorBlendAttachments& colorBlendAttachments) :
    attachments(colorBlendAttachments)
{
}

ColorBlendState::~ColorBlendState()
{
}

void ColorBlendState::read(Input& input)
{
    Object::read(input);

    logicOp = static_cast<VkLogicOp>(input.readValue<uint32_t>("logicOp"));
    logicOpEnable = static_cast<VkBool32>(input.readValue<uint32_t>("logicOpEnable"));

    attachments.resize(input.readValue<uint32_t>("NumColorBlendAttachments"));
    for (auto& colorBlendAttachment : attachments)
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

    // TODO : replace with vec4 IO
    input.read("blendConstants0", blendConstants[0]);
    input.read("blendConstants1", blendConstants[1]);
    input.read("blendConstants2", blendConstants[2]);
    input.read("blendConstants3", blendConstants[3]);
}

void ColorBlendState::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("logicOp", logicOp);
    output.writeValue<uint32_t>("logicOpEnable", logicOpEnable);

    output.writeValue<uint32_t>("NumColorBlendAttachments", attachments.size());
    for (auto& colorBlendAttachment : attachments)
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

void ColorBlendState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto colorBlendState = context.scratchMemory->allocate<VkPipelineColorBlendStateCreateInfo>();

    colorBlendState->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState->pNext = nullptr;
    colorBlendState->flags = 0;

    colorBlendState->logicOpEnable = logicOpEnable;
    colorBlendState->logicOp = logicOp;
    colorBlendState->attachmentCount = static_cast<uint32_t>(attachments.size());
    colorBlendState->pAttachments = attachments.data();
    colorBlendState->blendConstants[0] = blendConstants[0];
    colorBlendState->blendConstants[1] = blendConstants[1];
    colorBlendState->blendConstants[2] = blendConstants[2];
    colorBlendState->blendConstants[3] = blendConstants[3];

    pipelineInfo.pColorBlendState = colorBlendState;
}


////////////////////////////////////////////////////////////////////////
//
// DynamicState
//
DynamicState::DynamicState()
{
}

DynamicState::~DynamicState()
{
}

void DynamicState::read(Input& input)
{
    Object::read(input);

    dynamicStates.resize(input.readValue<uint32_t>("NumDynamicStates"));
    for (auto& dynamicState : dynamicStates)
    {
        input.readValue<uint32_t>("value", dynamicState);
    }
}

void DynamicState::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("NumDynamicStates", dynamicStates.size());
    for (auto& dynamicState : dynamicStates)
    {
        output.writeValue<uint32_t>("value", dynamicState);
    }
}

void DynamicState::apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    auto dynamicState = context.scratchMemory->allocate<VkPipelineDynamicStateCreateInfo>();

    dynamicState->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    dynamicState->pNext = nullptr;
    dynamicState->flags = 0;

    pipelineInfo.pDynamicState = dynamicState;
}
