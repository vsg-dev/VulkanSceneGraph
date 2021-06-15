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

GraphicsPipeline::GraphicsPipeline(PipelineLayout* in_pipelineLayout, const ShaderStages& in_shaderStages, const GraphicsPipelineStates& in_pipelineStates, uint32_t in_subpass) :
    stages(in_shaderStages),
    pipelineStates(in_pipelineStates),
    layout(in_pipelineLayout),
    subpass(in_subpass)
{
}

GraphicsPipeline::~GraphicsPipeline()
{
}

void GraphicsPipeline::read(Input& input)
{
    Object::read(input);

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("layout", layout);
        input.read("stages", stages);
        input.read("pipelineStates", pipelineStates);
    }
    else
    {
        input.read("PipelineLayout", layout);

        stages.resize(input.readValue<uint32_t>("NumShaderStages"));
        for (auto& shaderStage : stages)
        {
            input.read("ShaderStage", shaderStage);
        }

        pipelineStates.resize(input.readValue<uint32_t>("NumPipelineStates"));
        for (auto& pipelineState : pipelineStates)
        {
            input.read("PipelineState", pipelineState);
        }
    }

    input.read("subpass", subpass);
}

void GraphicsPipeline::write(Output& output) const
{
    Object::write(output);

    if (output.version_greater_equal(0, 1, 4))
    {
        output.write("layout", layout);
        output.write("stages", stages);
        output.write("pipelineStates", pipelineStates);
    }
    else
    {
        output.write("PipelineLayout", layout);

        output.writeValue<uint32_t>("NumShaderStages", stages.size());
        for (auto& shaderStage : stages)
        {
            output.write("ShaderStage", shaderStage);
        }

        output.writeValue<uint32_t>("NumPipelineStates", pipelineStates.size());
        for (auto& pipelineState : pipelineStates)
        {
            output.write("PipelineState", pipelineState);
        }
    }

    output.write("subpass", subpass);
}

void GraphicsPipeline::compile(Context& context)
{
    uint32_t viewID = context.viewID;
    if (static_cast<uint32_t>(_implementation.size()) < (viewID + 1))
    {
        _implementation.resize(viewID + 1);
    }

    if (!_implementation[viewID])
    {
        // compile shaders if required
        bool requiresShaderCompiler = false;
        for (auto& shaderStage : stages)
        {
            if (shaderStage->module)
            {
                if (shaderStage->module->code.empty() && !(shaderStage->module->source.empty()))
                {
                    requiresShaderCompiler = true;
                }
            }
        }

        if (requiresShaderCompiler)
        {
            auto shaderCompiler = context.getOrCreateShaderCompiler();
            if (shaderCompiler)
            {
                shaderCompiler->compile(stages); // may need to map defines and paths in some fashion
            }
        }

        // compile Vulkan objects
        layout->compile(context);

        for (auto& shaderStage : stages)
        {
            shaderStage->compile(context);
        }

        GraphicsPipelineStates combined_pipelineStates = context.defaultPipelineStates;
        combined_pipelineStates.insert(combined_pipelineStates.end(), pipelineStates.begin(), pipelineStates.end());
        combined_pipelineStates.insert(combined_pipelineStates.end(), context.overridePipelineStates.begin(), context.overridePipelineStates.end());

        _implementation[viewID] = GraphicsPipeline::Implementation::create(context, context.device, context.renderPass, layout, stages, combined_pipelineStates, subpass);
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
BindGraphicsPipeline::BindGraphicsPipeline(GraphicsPipeline* in_pipeline) :
    Inherit(0), // slot 0
    pipeline(in_pipeline)
{
}

BindGraphicsPipeline::~BindGraphicsPipeline()
{
}

void BindGraphicsPipeline::read(Input& input)
{
    StateCommand::read(input);

    if (input.version_greater_equal(0, 1, 4))
    {
        input.read("pipeline", pipeline);
    }
    else
    {
        input.read("GraphicsPipeline", pipeline);
    }
}

void BindGraphicsPipeline::write(Output& output) const
{
    StateCommand::write(output);

    if (output.version_greater_equal(0, 1, 4))
    {
        output.write("pipeline", pipeline);
    }
    else
    {
        output.write("GraphicsPipeline", pipeline);
    }
}

void BindGraphicsPipeline::record(CommandBuffer& commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk(commandBuffer.viewID));
    commandBuffer.setCurrentPipelineLayout(pipeline->layout);
}

void BindGraphicsPipeline::compile(Context& context)
{
    if (pipeline) pipeline->compile(context);
}

void BindGraphicsPipeline::release()
{
    if (pipeline) pipeline->release();
}
