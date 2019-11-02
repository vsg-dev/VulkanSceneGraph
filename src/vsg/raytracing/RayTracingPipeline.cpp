/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/raytracing/RayTracingPipeline.h>

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Extensions.h>
#include <vsg/traversals/CompileTraversal.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////
//
// RayTracingPipeline
//
RayTracingPipeline::RayTracingPipeline()
{
}

RayTracingPipeline::RayTracingPipeline(PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, RayTracingShaderBindings* shaderBindings, AllocationCallbacks* allocator) :
    _pipelineLayout(pipelineLayout),
    _shaderStages(shaderStages),
    _shaderBindings(shaderBindings),
    _allocator(allocator)
{
}

RayTracingPipeline::~RayTracingPipeline()
{
}

void RayTracingPipeline::read(Input& input)
{
    Object::read(input);

    _pipelineLayout = input.readObject<PipelineLayout>("PipelineLayout");

    _shaderStages.resize(input.readValue<uint32_t>("NumShaderStages"));
    for (auto& shaderStage : _shaderStages)
    {
        shaderStage = input.readObject<ShaderStage>("ShaderStage");
    }
}

void RayTracingPipeline::write(Output& output) const
{
    Object::write(output);

    output.writeObject("PipelineLayout", _pipelineLayout.get());

    output.writeValue<uint32_t>("NumShaderStages", _shaderStages.size());
    for (auto& shaderStage : _shaderStages)
    {
        output.writeObject("ShaderStage", shaderStage.get());
    }
}

void RayTracingPipeline::compile(Context& context)
{
    if (!_implementation)
    {
        _pipelineLayout->compile(context);

        for (auto& shaderStage : _shaderStages)
        {
            shaderStage->compile(context);
        }

        _implementation = RayTracingPipeline::Implementation::create(context.device, _pipelineLayout, _shaderStages, _shaderBindings, _allocator);
        _shaderBindings->compile(context, _implementation->_pipeline); // NEED TO COMPILE SHADER BINDINGS AFTER PIPELINE IS CREATED
    }
}

////////////////////////////////////////////////////////////////////////
//
// RayTracingPipeline::Implementation
//
RayTracingPipeline::Implementation::Implementation(VkPipeline pipeline, Device* device, PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, RayTracingShaderBindings* shaderBindings, AllocationCallbacks* allocator) :
    _pipeline(pipeline),
    _device(device),
    _pipelineLayout(pipelineLayout),
    _shaderStages(shaderStages),
    _shaderBindings(shaderBindings),
    _allocator(allocator)
{
}

RayTracingPipeline::Implementation::Result RayTracingPipeline::Implementation::create(Device* device, PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, RayTracingShaderBindings* shaderBindings, AllocationCallbacks* allocator)
{
    if (!device || !pipelineLayout)
    {
        return Result("Error: vsg::RayTracingPipeline::create(...) failed to create raytracing pipeline, inputs not defined.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    Extensions* extensions = Extensions::Get(device, true);

    VkRayTracingPipelineCreateInfoNV pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
    pipelineInfo.layout = *pipelineLayout;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pNext = nullptr;

    std::vector<VkSpecializationInfo> specializationInfos(shaderStages.size());
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo(shaderStages.size());
    for (size_t i = 0; i < shaderStages.size(); ++i)
    {
        const ShaderStage* shaderStage = shaderStages[i];
        shaderStageCreateInfo[i].pNext = nullptr;
        shaderStage->apply(shaderStageCreateInfo[i]);
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

    pipelineInfo.groupCount = static_cast<uint32_t>(shaderBindings->createInfos().size());
    pipelineInfo.pGroups = shaderBindings->createInfos().data();

    pipelineInfo.maxRecursionDepth = 1;


    VkPipeline pipeline;
    VkResult result = extensions->vkCreateRayTracingPipelinesNV(*device, VK_NULL_HANDLE, 1, &pipelineInfo, allocator, &pipeline);
    if (result == VK_SUCCESS)
    {
        return Result(new Implementation(pipeline, device, pipelineLayout, shaderStages, shaderBindings, allocator));
    }
    else
    {
        return Result("Error: vsg::Pipeline::createGraphics(...) failed to create VkPipeline.", result);
    }
}

RayTracingPipeline::Implementation::~Implementation()
{
    vkDestroyPipeline(*_device, _pipeline, _allocator);
}

////////////////////////////////////////////////////////////////////////
//
// BindRayTracingPipeline
//
BindRayTracingPipeline::BindRayTracingPipeline(RayTracingPipeline* pipeline) :
    Inherit(0), // slot 0
    _pipeline(pipeline)
{
}

BindRayTracingPipeline::~BindRayTracingPipeline()
{
}

void BindRayTracingPipeline::read(Input& input)
{
    StateCommand::read(input);

    _pipeline = input.readObject<RayTracingPipeline>("RayTracingPipeline");
}

void BindRayTracingPipeline::write(Output& output) const
{
    StateCommand::write(output);

    output.writeObject("RayTracingPipeline", _pipeline.get());
}

void BindRayTracingPipeline::dispatch(CommandBuffer& commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, *_pipeline);
    commandBuffer.setCurrentPipelineLayout(*(_pipeline->getPipelineLayout()));
}

void BindRayTracingPipeline::compile(Context& context)
{
    if (_pipeline) _pipeline->compile(context);
}

void BindRayTracingPipeline::release()
{
    if (_pipeline) _pipeline->release();
}