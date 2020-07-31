/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Options.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////
//
// GraphicsPipeline
//
GraphicsPipeline::GraphicsPipeline()
{
}

GraphicsPipeline::GraphicsPipeline(PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const GraphicsPipelineStates& pipelineStates, uint32_t subpass) :
    _pipelineLayout(pipelineLayout),
    _shaderStages(shaderStages),
    _pipelineStates(pipelineStates),
    _subpass(subpass)
{
}

GraphicsPipeline::~GraphicsPipeline()
{
}

void GraphicsPipeline::read(Input& input)
{
    Object::read(input);

    input.readObject("PipelineLayout", _pipelineLayout);

    _shaderStages.resize(input.readValue<uint32_t>("NumShaderStages"));
    for (auto& shaderStage : _shaderStages)
    {
        input.readObject("ShaderStage", shaderStage);
    }

    _pipelineStates.resize(input.readValue<uint32_t>("NumPipelineStates"));
    for (auto& pipelineState : _pipelineStates)
    {
        input.readObject("PipelineState", pipelineState);
    }

    input.read("subpass", _subpass);
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

    output.write("subpass", _subpass);
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

        GraphicsPipelineStates combined_pipelineStates = context.defaultPipelineStates;
        combined_pipelineStates.insert(combined_pipelineStates.end(), _pipelineStates.begin(), _pipelineStates.end());
        combined_pipelineStates.insert(combined_pipelineStates.end(), context.overridePipelineStates.begin(), context.overridePipelineStates.end());

        // TODO : current buffering of GraphicsPipeline::Implementation assumes a single VkPipelie for each Device, but could potentially vary with Device, RenerPass, combined_PipelinStates
        // so will need to have some form of contextID/renderID that wraps all these variables up and indexes the buffer using it instead of deviceID.
        _implementation[context.deviceID] = GraphicsPipeline::Implementation::create(context, context.device, context.renderPass, _pipelineLayout, _shaderStages, combined_pipelineStates, _subpass);
    }
}

////////////////////////////////////////////////////////////////////////
//
// GraphicsPipeline::Implementation
//
GraphicsPipeline::Implementation::Implementation(Context& context, Device* device, RenderPass* renderPass, PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const GraphicsPipelineStates& pipelineStates, uint32_t subpass) :
    _device(device)
{
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = pipelineLayout->vk(device->deviceID);
    pipelineInfo.renderPass = *renderPass;
    pipelineInfo.subpass = subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pNext = nullptr;

    auto shaderStageCreateInfo = context.scratchMemory->allocate<VkPipelineShaderStageCreateInfo>(shaderStages.size());
    for (size_t i = 0; i < shaderStages.size(); ++i)
    {
        const ShaderStage* shaderStage = shaderStages[i];
        shaderStageCreateInfo[i].flags = 0;
        shaderStageCreateInfo[i].pNext = nullptr;
        shaderStage->apply(context, shaderStageCreateInfo[i]);
    }

    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStageCreateInfo;

    for (auto pipelineState : pipelineStates)
    {
        pipelineState->apply(context, pipelineInfo);
    }

    VkResult result = vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, _device->getAllocationCallbacks(), &_pipeline);

    context.scratchMemory->release();

    if (result != VK_SUCCESS)
    {
        throw Exception{"Error: vsg::Pipeline::createGraphics(...) failed to create VkPipeline.", result};
    }
}

GraphicsPipeline::Implementation::~Implementation()
{
    vkDestroyPipeline(*_device, _pipeline, _device->getAllocationCallbacks());
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

    input.readObject("GraphicsPipeline", _pipeline);
}

void BindGraphicsPipeline::write(Output& output) const
{
    StateCommand::write(output);

    output.writeObject("GraphicsPipeline", _pipeline.get());
}

void BindGraphicsPipeline::record(CommandBuffer& commandBuffer) const
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
