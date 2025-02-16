/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/io/Logger.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/ViewportState.h>
#include <vsg/vk/Context.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////
//
// GraphicsPipelineState
//
int GraphicsPipelineState::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_value(mask, rhs.mask);
}

void GraphicsPipelineState::read(Input& input)
{
    Object::read(input);

    if (input.version_greater_equal(1, 0, 9))
    {
        input.read("mask", mask);
    }
}

void GraphicsPipelineState::write(Output& output) const
{
    Object::write(output);

    if (output.version_greater_equal(1, 0, 9))
    {
        output.write("mask", mask);
    }
}

void vsg::mergeGraphicsPipelineStates(Mask mask, GraphicsPipelineStates& dest_PipelineStates, ref_ptr<GraphicsPipelineState> src_PipelineState)
{
    if (!src_PipelineState || (mask & src_PipelineState->mask) == 0) return;

    // replace any entries in the dest_PipelineStates that have the same type as src_PipelineState
    for (auto& original_pipelineState : dest_PipelineStates)
    {
        if (original_pipelineState->type_info() == src_PipelineState->type_info())
        {
            original_pipelineState = src_PipelineState;
            return;
        }
    }
    dest_PipelineStates.push_back(src_PipelineState);
}

void vsg::mergeGraphicsPipelineStates(Mask mask, GraphicsPipelineStates& dest_PipelineStates, const GraphicsPipelineStates& src_PipelineStates)
{
    for (const auto& src_PipelineState : src_PipelineStates)
    {
        mergeGraphicsPipelineStates(mask, dest_PipelineStates, src_PipelineState);
    }
}

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

int GraphicsPipeline::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_pointer_container(stages, rhs.stages))) return result;
    if ((result = compare_pointer_container(pipelineStates, rhs.pipelineStates))) return result;
    if ((result = compare_pointer(layout, rhs.layout))) return result;
    return compare_value(subpass, rhs.subpass);
}

void GraphicsPipeline::read(Input& input)
{
    Object::read(input);

    input.readObject("layout", layout);
    input.readObjects("stages", stages);
    input.readObjects("pipelineStates", pipelineStates);
    input.read("subpass", subpass);
}

void GraphicsPipeline::write(Output& output) const
{
    Object::write(output);

    output.writeObject("layout", layout);
    output.writeObjects("stages", stages);
    output.writeObjects("pipelineStates", pipelineStates);
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
        GraphicsPipelineStates combined_pipelineStates;
        combined_pipelineStates.reserve(context.defaultPipelineStates.size() + pipelineStates.size() + context.overridePipelineStates.size());
        mergeGraphicsPipelineStates(context.mask, combined_pipelineStates, context.defaultPipelineStates);
        mergeGraphicsPipelineStates(context.mask, combined_pipelineStates, pipelineStates);
        mergeGraphicsPipelineStates(context.mask, combined_pipelineStates, context.overridePipelineStates);

        for (const auto& imp : _implementation)
        {
            if (imp && vsg::compare_pointer_container(imp->_pipelineStates, combined_pipelineStates) == 0)
            {
                _implementation[viewID] = imp;
                return;
            }
        }

        // compile shaders if required
        bool requiresShaderCompiler = false;
        for (const auto& shaderStage : stages)
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
            else
            {
                fatal("VulkanSceneGraph not compiled with GLSLang, unable to compile shaders.");
            }
        }

        // compile Vulkan objects
        layout->compile(context);

        for (auto& shaderStage : stages)
        {
            shaderStage->compile(context);
        }

        _implementation[viewID] = GraphicsPipeline::Implementation::create(context, context.device, context.renderPass, layout, stages, combined_pipelineStates, subpass);
    }
}

////////////////////////////////////////////////////////////////////////
//
// GraphicsPipeline::Implementation
//
GraphicsPipeline::Implementation::Implementation(Context& context, Device* device, const RenderPass* renderPass, const PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const GraphicsPipelineStates& pipelineStates, uint32_t subpass) :
    _pipelineStates(pipelineStates),
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
    uint32_t i = 0;
    for (auto& shaderStage : shaderStages)
    {
        // check if ShaderStage is appropriate to assign to stageInfo
        if ((context.mask & shaderStage->mask) != 0)
        {
            shaderStageCreateInfo[i].flags = 0;
            shaderStageCreateInfo[i].pNext = nullptr;
            shaderStage->apply(context, shaderStageCreateInfo[i]);
            ++i;
        }
    }

    pipelineInfo.stageCount = i;
    pipelineInfo.pStages = shaderStageCreateInfo;

    for (auto pipelineState : pipelineStates)
    {
        pipelineState->apply(context, pipelineInfo);
    }

    VkResult result = vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, _device->getAllocationCallbacks(), &_pipeline);

    context.scratchMemory->release();

    if (result != VK_SUCCESS)
    {
        throw Exception{"Error: vsg::GraphicsPipeline failed to create VkPipeline.", result};
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
BindGraphicsPipeline::BindGraphicsPipeline(ref_ptr<GraphicsPipeline> in_pipeline) :
    Inherit(0), // slot 0
    pipeline(in_pipeline)
{
}

BindGraphicsPipeline::~BindGraphicsPipeline()
{
}

int BindGraphicsPipeline::compare(const Object& rhs_object) const
{
    int result = StateCommand::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_pointer(pipeline, rhs.pipeline);
}

void BindGraphicsPipeline::read(Input& input)
{
    StateCommand::read(input);

    input.readObject("pipeline", pipeline);
}

void BindGraphicsPipeline::write(Output& output) const
{
    StateCommand::write(output);

    output.writeObject("pipeline", pipeline);
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
