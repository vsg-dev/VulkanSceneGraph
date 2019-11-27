/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <algorithm>

#include <vsg/raytracing/AccelerationStructure.h>

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/Extensions.h>

using namespace vsg;

AccelerationStructure::AccelerationStructure(VkAccelerationStructureTypeNV type, Device* device, Allocator* allocator) :
    Inherit(allocator),
    _device(device),
    _requiredBuildScratchSize(0)
{
    _accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    _accelerationStructureInfo.type = type;
    _accelerationStructureInfo.flags = 0; // probably be useful to set this somehow
    _accelerationStructureInfo.instanceCount = 0;
    _accelerationStructureInfo.geometryCount = 0;
    _accelerationStructureInfo.pGeometries = nullptr;
    _accelerationStructureInfo.pNext = nullptr;
}

AccelerationStructure::~AccelerationStructure()
{
    if (_accelerationStructure)
    {
        Extensions* extensions = Extensions::Get(_device, true);
        extensions->vkDestroyAccelerationStructureNV(*_device, _accelerationStructure, nullptr);
    }
}

void AccelerationStructure::compile(Context& context)
{
    Extensions* extensions = Extensions::Get(context.device, true);

    VkAccelerationStructureCreateInfoNV createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    createInfo.info = _accelerationStructureInfo;

    VkResult result = extensions->vkCreateAccelerationStructureNV(*context.device, &createInfo, nullptr, &_accelerationStructure);

    if (result == VK_SUCCESS)
    {
        VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo{};
        memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
        memoryRequirementsInfo.accelerationStructure = _accelerationStructure;

        VkMemoryRequirements2 memoryRequirements2{};
        extensions->vkGetAccelerationStructureMemoryRequirementsNV(*context.device, &memoryRequirementsInfo, &memoryRequirements2);

        _memory = DeviceMemory::create(context.device, memoryRequirements2.memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo{};
        accelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
        accelerationStructureMemoryInfo.accelerationStructure = _accelerationStructure;
        accelerationStructureMemoryInfo.memory = *_memory;
        result = extensions->vkBindAccelerationStructureMemoryNV(*context.device, 1, &accelerationStructureMemoryInfo);

        result = extensions->vkGetAccelerationStructureHandleNV(*context.device, _accelerationStructure, sizeof(uint64_t), &_handle);

        _requiredBuildScratchSize = memoryRequirements2.memoryRequirements.size;
        context.scratchBufferSize = std::max(_requiredBuildScratchSize, context.scratchBufferSize);
    }
    else
    {
        // error
    }
}

void AccelerationStructure::dispatch(CommandBuffer& commandBuffer) const
{
}
