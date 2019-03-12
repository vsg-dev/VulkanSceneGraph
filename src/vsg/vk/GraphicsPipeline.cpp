/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/GraphicsPipeline.h>
#include <vsg/vk/State.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////
//
// GraphicsPipeline
//
GraphicsPipeline::GraphicsPipeline()
{
}

GraphicsPipeline::GraphicsPipeline(PipelineLayout* pipelineLayout, const GraphicsPipelineStates& pipelineStates, AllocationCallbacks* allocator) :
    _pipelineLayout(pipelineLayout),
    _pipelineStates(pipelineStates),
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

    _pipelineStates.resize(input.readValue<uint32_t>("NumPipelineStates"));
    for (auto& pipelineState : _pipelineStates)
    {
        pipelineState = input.readObject<GraphicsPipelineState>("PipelineState");
    }
}

void GraphicsPipeline::write(Output& output) const
{
    Object::write(output);

    output.writeObject("PipelineLayout", _pipelineLayout.get());

    output.writeValue<uint32_t>("NumPipelineStates", _pipelineStates.size());
    for (auto& pipelineState : _pipelineStates)
    {
        output.writeObject("PipelineState", pipelineState.get());
    }
}

void GraphicsPipeline::compile(Context& context)
{
    if (!_implementation)
    {
        _pipelineLayout->compile(context);

        for (auto& pipelineState : _pipelineStates)
        {
            pipelineState->compile(context);
        }

        GraphicsPipelineStates full_pipelineStates = _pipelineStates;
        full_pipelineStates.emplace_back(context.viewport);

        _implementation = GraphicsPipeline::Implementation::create(context.device, context.renderPass, _pipelineLayout, full_pipelineStates, _allocator);
    }
}

////////////////////////////////////////////////////////////////////////
//
// GraphicsPipeline::Implementation
//
GraphicsPipeline::Implementation::Implementation(VkPipeline pipeline, Device* device, RenderPass* renderPass, PipelineLayout* pipelineLayout, const GraphicsPipelineStates& pipelineStates, AllocationCallbacks* allocator) :
    _pipeline(pipeline),
    _device(device),
    _renderPass(renderPass),
    _pipelineLayout(pipelineLayout),
    _pipelineStates(pipelineStates),
    _allocator(allocator)
{
}

GraphicsPipeline::Implementation::Result GraphicsPipeline::Implementation::create(Device* device, RenderPass* renderPass, PipelineLayout* pipelineLayout, const GraphicsPipelineStates& pipelineStates, AllocationCallbacks* allocator)
{
    if (!device || !renderPass || !pipelineLayout)
    {
        return Result("Error: vsg::GraphicsPipeline::create(...) failed to create graphics pipeline, inputs not defined.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
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
    VkResult result = vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, allocator, &pipeline);
    if (result == VK_SUCCESS)
    {
        return Result(new Implementation(pipeline, device, renderPass, pipelineLayout, pipelineStates, allocator));
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

void BindGraphicsPipeline::pushTo(State& state) const
{
    state.dirty = true;
    state.graphicsPipelineStack.push(this);
}

void BindGraphicsPipeline::popFrom(State& state) const
{
    state.dirty = true;
    state.graphicsPipelineStack.pop();
}

void BindGraphicsPipeline::dispatch(CommandBuffer& commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *_pipeline);
    commandBuffer.setCurrentPipelineLayout(_pipeline->getPipelineLayout());
}

void BindGraphicsPipeline::compile(Context& context)
{
    if (_pipeline) _pipeline->compile(context);
}

////////////////////////////////////////////////////////////////////////
//
// ShaderStages
//
ShaderStages::ShaderStages()
{
}

ShaderStages::ShaderStages(const ShaderModules& shaderModules)
{
    setShaderModules(shaderModules);
}

ShaderStages::~ShaderStages()
{
}

void ShaderStages::read(Input& input)
{
    Object::read(input);

    _shaderModules.resize(input.readValue<uint32_t>("NumShaderModule"));
    for (auto& shaderModule : _shaderModules)
    {
        shaderModule = input.readObject<ShaderModule>("ShaderModule");
    }
}

void ShaderStages::write(Output& output) const
{
    Object::write(output);

    output.writeValue<uint32_t>("NumShaderModule", _shaderModules.size());
    for (auto& shaderModule : _shaderModules)
    {
        output.writeObject("ShaderModule", shaderModule.get());
    }
}

void ShaderStages::apply(VkGraphicsPipelineCreateInfo& pipelineInfo) const
{
    pipelineInfo.stageCount = static_cast<uint32_t>(size());
    pipelineInfo.pStages = data();
}

void ShaderStages::compile(Context& context)
{
    _stages.resize(_shaderModules.size());
    for (size_t i = 0; i < _shaderModules.size(); ++i)
    {
        VkPipelineShaderStageCreateInfo& stageInfo = (_stages)[i];
        ShaderModule* sm = _shaderModules[i];
        sm->compile(context);
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage = sm->stage();
        stageInfo.module = *sm;
        stageInfo.pName = sm->entryPointName().c_str();
    }
}

void ShaderStages::release()
{
    for (auto& shaderModules : _shaderModules)
    {
        shaderModules->release();
    }

    _stages.clear();
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
}

VertexInputState::VertexInputState(const Bindings& bindings, const Attributes& attributes) :
    VkPipelineVertexInputStateCreateInfo{},
    _bindings(bindings),
    _attributes(attributes)
{
    sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
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
}

InputAssemblyState::~InputAssemblyState()
{
}

void InputAssemblyState::read(Input& input)
{
    Object::read(input);
}

void InputAssemblyState::write(Output& output) const
{
    Object::write(output);
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
}

ViewportState::ViewportState(const VkExtent2D& extent) :
    VkPipelineViewportStateCreateInfo{},
    _viewport{},
    _scissor{}
{
    sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
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
}

DepthStencilState::~DepthStencilState()
{
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
}

ColorBlendState::~ColorBlendState()
{
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
