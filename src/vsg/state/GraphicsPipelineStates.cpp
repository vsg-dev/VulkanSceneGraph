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

InputAssemblyState::InputAssemblyState(VkPrimitiveTopology primitiveTopology, VkBool32 primitiveRestart):
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
    inputAssemblyState->primitiveRestartEnable =  primitiveRestartEnable;

    pipelineInfo.pInputAssemblyState = inputAssemblyState;
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
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    viewports.push_back(viewport);

    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = extent;
    scissors.push_back(scissor);
}

ViewportState::~ViewportState()
{
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
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT;

    attachments.push_back(colorBlendAttachment);
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
