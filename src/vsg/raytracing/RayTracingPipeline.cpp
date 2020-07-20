/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/raytracing/RayTracingPipeline.h>

#include <vsg/core/Exception.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Extensions.h>
#include <vsg/io/Options.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////
//
// RayTracingPipeline
//
RayTracingPipeline::RayTracingPipeline()
{
}

RayTracingPipeline::RayTracingPipeline(PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const RayTracingShaderGroups& shaderGroups, AllocationCallbacks* allocator) :
    _pipelineLayout(pipelineLayout),
    _shaderStages(shaderStages),
    _rayTracingShaderGroups(shaderGroups),
    _allocator(allocator)
{
}

RayTracingPipeline::~RayTracingPipeline()
{
}

void RayTracingPipeline::read(Input& input)
{
    Object::read(input);

    input.readObject("PipelineLayout", _pipelineLayout);

    _shaderStages.resize(input.readValue<uint32_t>("NumShaderStages"));
    for (auto& shaderStage : _shaderStages)
    {
        input.readObject("ShaderStage", shaderStage);
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
    if (!_implementation[context.deviceID])
    {
        _pipelineLayout->compile(context);

        for (auto& shaderStage : _shaderStages)
        {
            shaderStage->compile(context);
        }

        _implementation[context.deviceID] = RayTracingPipeline::Implementation::create(context, this);
    }
}

////////////////////////////////////////////////////////////////////////
//
// RayTracingPipeline::Implementation
//
RayTracingPipeline::Implementation::Implementation(Context& context, RayTracingPipeline* rayTracingPipeline) :
    _device(context.device),
    _pipelineLayout(rayTracingPipeline->getPipelineLayout()),
    _shaderStages(rayTracingPipeline->getShaderStages()),
    _shaderGroups(rayTracingPipeline->getRayTracingShaderGroups()),
    _allocator(rayTracingPipeline->getAllocationCallbacks())
{

    auto pipelineLayout = rayTracingPipeline->getPipelineLayout();

    Extensions* extensions = Extensions::Get(_device, true);

    VkRayTracingPipelineCreateInfoNV pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
    pipelineInfo.layout = pipelineLayout->vk(context.deviceID);
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pNext = nullptr;

    auto shaderStages = rayTracingPipeline->getShaderStages();

    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo(shaderStages.size());
    for (size_t i = 0; i < shaderStages.size(); ++i)
    {
        const ShaderStage* shaderStage = shaderStages[i];
        shaderStageCreateInfo[i].pNext = nullptr;
        shaderStage->apply(context, shaderStageCreateInfo[i]);
    }

    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStageCreateInfo.size());
    pipelineInfo.pStages = shaderStageCreateInfo.data();

    // assign the RayTracingShaderGroups
    auto& rayTracingShaderGroups = rayTracingPipeline->getRayTracingShaderGroups();
    std::vector<VkRayTracingShaderGroupCreateInfoNV> shaderGroups(rayTracingShaderGroups.size());
    for (size_t i = 0; i < rayTracingShaderGroups.size(); ++i)
    {
        rayTracingShaderGroups[i]->applyTo(shaderGroups[i]);
    }

    pipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
    pipelineInfo.pGroups = shaderGroups.data();

    pipelineInfo.maxRecursionDepth = rayTracingPipeline->maxRecursionDepth();

    VkResult result = extensions->vkCreateRayTracingPipelinesNV(*_device, VK_NULL_HANDLE, 1, &pipelineInfo, rayTracingPipeline->getAllocationCallbacks(), &_pipeline);
    if (result == VK_SUCCESS)
    {
        auto rayTracingProperties = _device->getPhysicalDevice()->getProperties<VkPhysicalDeviceRayTracingPropertiesNV, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV>();
        const uint32_t shaderGroupHandleSize = rayTracingProperties.shaderGroupHandleSize;
        const uint32_t sbtSize = shaderGroupHandleSize * pipelineInfo.groupCount;

        BufferData bindingTableBufferData = context.stagingMemoryBufferPools->reserveBufferData(sbtSize, 4, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        auto bindingTableBuffer = bindingTableBufferData._buffer;
        auto bindingTableMemory = bindingTableBuffer->getDeviceMemory();

        void* buffer_data;
        bindingTableMemory->map(bindingTableBuffer->getMemoryOffset() + bindingTableBufferData._offset, bindingTableBufferData._range, 0, &buffer_data);

        extensions->vkGetRayTracingShaderGroupHandlesNV(*_device, _pipeline, 0, static_cast<uint32_t>(rayTracingShaderGroups.size()), sbtSize, buffer_data);

        bindingTableMemory->unmap();

        VkDeviceSize offset = bindingTableBufferData._offset;

        for (size_t i = 0; i < rayTracingShaderGroups.size(); ++i)
        {
            rayTracingShaderGroups[i]->bufferData._buffer = bindingTableBuffer;
            rayTracingShaderGroups[i]->bufferData._offset = offset;

            offset += shaderGroupHandleSize;
        }
    }
    else
    {
        throw Exception{"Error: vsg::Pipeline::createGraphics(...) failed to create VkPipeline.", result};
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

    input.readObject("RayTracingPipeline", _pipeline);
}

void BindRayTracingPipeline::write(Output& output) const
{
    StateCommand::write(output);

    output.writeObject("RayTracingPipeline", _pipeline.get());
}

void BindRayTracingPipeline::record(CommandBuffer& commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, _pipeline->vk(commandBuffer.deviceID));
    commandBuffer.setCurrentPipelineLayout(_pipeline->getPipelineLayout()->vk(commandBuffer.deviceID));
}

void BindRayTracingPipeline::compile(Context& context)
{
    if (_pipeline) _pipeline->compile(context);
}

void BindRayTracingPipeline::release()
{
    if (_pipeline) _pipeline->release();
}
