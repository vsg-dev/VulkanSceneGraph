/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <algorithm>

#include <vsg/core/Exception.h>
#include <vsg/raytracing/AccelerationStructure.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>

using namespace vsg;

AccelerationStructure::AccelerationStructure(VkAccelerationStructureTypeKHR type, Device* device) :
    _accelerationStructure{},
    _accelerationStructureInfo{},
    _accelerationStructureBuildGeometryInfo{},
    _requiredBuildScratchSize(0),
    _device(device)
{
    _accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    _accelerationStructureInfo.type = type;
    _accelerationStructureInfo.createFlags = 0; // probably be useful to set this somehow
    _accelerationStructureInfo.buffer = 0;
    _accelerationStructureInfo.deviceAddress = 0;
    _accelerationStructureInfo.offset = 0;
    _accelerationStructureInfo.size = 0;
    _accelerationStructureInfo.pNext = nullptr;

    _accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    _accelerationStructureBuildGeometryInfo.type = type;
    _accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
}

AccelerationStructure::~AccelerationStructure()
{
    if (_accelerationStructure)
    {
        auto extensions = _device->getExtensions();
        extensions->vkDestroyAccelerationStructureKHR(*_device, _accelerationStructure, nullptr);
    }
}

void AccelerationStructure::compile(Context& context)
{
    auto extensions = context.device->getExtensions();

    VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
    accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    extensions->vkGetAccelerationStructureBuildSizesKHR(
        *context.device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &_accelerationStructureBuildGeometryInfo,
        _geometryPrimitiveCounts.data(),
        &accelerationStructureBuildSizesInfo);

    _buffer = vsg::createBufferAndMemory(context.device, accelerationStructureBuildSizesInfo.accelerationStructureSize,
                                         VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    _accelerationStructureInfo.buffer = _buffer->vk(context.deviceID);
    _accelerationStructureInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
    VkResult result = extensions->vkCreateAccelerationStructureKHR(*context.device, &_accelerationStructureInfo, nullptr, &_accelerationStructure);
    if (result == VK_SUCCESS)
    {
        VkAccelerationStructureDeviceAddressInfoKHR deviceAddressInfo{};
        deviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        deviceAddressInfo.accelerationStructure = _accelerationStructure;
        _handle = extensions->vkGetAccelerationStructureDeviceAddressKHR(*context.device, &deviceAddressInfo);

        _requiredBuildScratchSize = accelerationStructureBuildSizesInfo.buildScratchSize;
        context.scratchBufferSize = std::max(_requiredBuildScratchSize, context.scratchBufferSize);
    }
    else
    {
        throw Exception{"Error: vsg::AccelerationStructure::compile(...) failed to create AccelerationStructure.", result};
    }
}
