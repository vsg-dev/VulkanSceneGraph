/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/io/Logger.h>
#include <vsg/raytracing/RayTracingPipeline.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////
//
// RayTracingPipeline
//
RayTracingPipeline::RayTracingPipeline()
{
}

RayTracingPipeline::RayTracingPipeline(PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const RayTracingShaderGroups& shaderGroups) :
    _pipelineLayout(pipelineLayout),
    _shaderStages(shaderStages),
    _rayTracingShaderGroups(shaderGroups)
{
}

RayTracingPipeline::~RayTracingPipeline()
{
}

int RayTracingPipeline::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_pointer_container(_shaderStages, rhs._shaderStages))) return result;
    if ((result = compare_pointer_container(_rayTracingShaderGroups, rhs._rayTracingShaderGroups))) return result;
    if ((result = compare_pointer(_pipelineLayout, rhs._pipelineLayout))) return result;
    return compare_value(_maxRecursionDepth, rhs._maxRecursionDepth);
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
    for (const auto& shaderStage : _shaderStages)
    {
        output.writeObject("ShaderStage", shaderStage.get());
    }
}

void RayTracingPipeline::compile(Context& context)
{
    if (!_implementation[context.deviceID])
    {
        // compile shaders if required
        bool requiresShaderCompiler = false;
        for (const auto& shaderStage : _shaderStages)
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
                shaderCompiler->compile(_shaderStages); // may need to map defines and paths in some fashion
            }
            else
            {
                fatal("VulkanSceneGraph not compiled with GLSLang, unable to compile shaders.");
            }
        }

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
    _shaderGroups(rayTracingPipeline->getRayTracingShaderGroups())
{

    auto pipelineLayout = rayTracingPipeline->getPipelineLayout();

    auto extensions = _device->getExtensions();

    VkRayTracingPipelineCreateInfoKHR pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
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
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups(rayTracingShaderGroups.size());
    for (size_t i = 0; i < rayTracingShaderGroups.size(); ++i)
    {
        rayTracingShaderGroups[i]->applyTo(shaderGroups[i]);
    }

    pipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
    pipelineInfo.pGroups = shaderGroups.data();

    pipelineInfo.maxPipelineRayRecursionDepth = rayTracingPipeline->maxRecursionDepth();

    VkResult result = extensions->vkCreateRayTracingPipelinesKHR(*_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineInfo, _device->getAllocationCallbacks(), &_pipeline);
    if (result == VK_SUCCESS)
    {
        auto rayTracingProperties = _device->getPhysicalDevice()->getProperties<VkPhysicalDeviceRayTracingPipelinePropertiesKHR, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR>();
        auto alignedSize = [](uint32_t value, uint32_t alignment) {
            return (value + alignment - 1) & ~(alignment - 1);
        };
        const uint32_t handleSizeAligned = alignedSize(rayTracingProperties.shaderGroupHandleSize, rayTracingProperties.shaderGroupHandleAlignment);
        const uint32_t sbtSize = handleSizeAligned * pipelineInfo.groupCount;

        //BufferInfo bindingTableBufferInfo = context.stagingMemoryBufferPools->reserveBuffer(sbtSize, 4, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        //auto bindingTableBuffer = bindingTableBufferInfo.buffer;
        //auto bindingTableMemory = bindingTableBuffer->getDeviceMemory(context.deviceID);
        std::vector<ref_ptr<Buffer>> bindingTableBuffers(rayTracingShaderGroups.size());
        for (size_t i = 0; i < bindingTableBuffers.size(); ++i)
        {
            bindingTableBuffers[i] = createBufferAndMemory(_device, handleSizeAligned, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        }

        std::vector<uint8_t> shaderHandleStorage(sbtSize);
        extensions->vkGetRayTracingShaderGroupHandlesKHR(*_device, _pipeline, 0, static_cast<uint32_t>(rayTracingShaderGroups.size()), sbtSize, shaderHandleStorage.data());

        for (size_t i = 0; i < rayTracingShaderGroups.size(); ++i)
        {
            auto memory = bindingTableBuffers[i]->getDeviceMemory(context.deviceID);
            void* data;
            memory->map(bindingTableBuffers[i]->getMemoryOffset(_device->deviceID), handleSizeAligned, 0, &data);
            memcpy(data, shaderHandleStorage.data() + i * handleSizeAligned, handleSizeAligned);
            memory->unmap();
            rayTracingShaderGroups[i]->bufferInfo = vsg::BufferInfo::create();
            rayTracingShaderGroups[i]->bufferInfo->buffer = bindingTableBuffers[i];
            rayTracingShaderGroups[i]->bufferInfo->offset = 0;
            rayTracingShaderGroups[i]->bufferInfo->range = handleSizeAligned;
        }
    }
    else
    {
        throw Exception{"Error: vsg::RayTracingPipeline failed to create VkPipeline.", result};
    }
}

RayTracingPipeline::Implementation::~Implementation()
{
    vkDestroyPipeline(*_device, _pipeline, _device->getAllocationCallbacks());
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

int BindRayTracingPipeline::compare(const Object& rhs_object) const
{
    int result = StateCommand::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    return compare_pointer(_pipeline, rhs._pipeline);
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
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, _pipeline->vk(commandBuffer.deviceID));
    commandBuffer.setCurrentPipelineLayout(_pipeline->getPipelineLayout());
}

void BindRayTracingPipeline::compile(Context& context)
{
    if (_pipeline) _pipeline->compile(context);
}

void BindRayTracingPipeline::release()
{
    if (_pipeline) _pipeline->release();
}
