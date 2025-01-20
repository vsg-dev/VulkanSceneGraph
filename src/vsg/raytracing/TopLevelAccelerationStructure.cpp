/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <algorithm>

#include <vsg/raytracing/TopLevelAccelerationStructure.h>

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>

using namespace vsg;

#define TRANSFER_BUFFERS 0

GeometryInstance::GeometryInstance() :
    id(0),
    mask(0xff),
    shaderOffset(0),
    flags(VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR)
{
}

TopLevelAccelerationStructure::TopLevelAccelerationStructure(Device* device) :
    Inherit(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, device)
{
}

void TopLevelAccelerationStructure::compile(Context& context)
{
    if (geometryInstances.empty()) return; // no data
    if (_instances) return;                // already compiled

    // allocate instances array to size of reference bottom level geoms list
    _instances = VkGeometryInstanceArray::create(static_cast<uint32_t>(geometryInstances.size()));

    // compile the referenced bottom level acceleration structures and add geom instance to instances array
    for (uint32_t i = 0; i < geometryInstances.size(); i++)
    {
        geometryInstances[i]->accelerationStructure->compile(context);
        _instances->set(i, *geometryInstances[i]);
    }

    DataList dataList = {_instances};

#if TRANSFER_BUFFERS
    auto instanceBufferInfo = vsg::createBufferAndTransferData(context, dataList, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_SHARING_MODE_EXCLUSIVE);
    _instanceBuffer = instanceBufferInfo[0].buffer;
#else
    auto instanceBufferInfo = vsg::createHostVisibleBuffer(context.device, dataList, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_SHARING_MODE_EXCLUSIVE);
    vsg::copyDataListToBuffers(context.device, instanceBufferInfo);
    _instanceBuffer = instanceBufferInfo[0]->buffer;
#endif
    auto extensions = _device->getExtensions();
    VkBufferDeviceAddressInfo bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, _instanceBuffer->vk(context.deviceID)};
    VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
    accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
    accelerationStructureGeometry.geometry.instances.data.deviceAddress = extensions->vkGetBufferDeviceAddressKHR(*context.device, &bufferDeviceAddressInfo);

    _accelerationStructureBuildGeometryInfo.geometryCount = 1;
    _accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
    _geometryPrimitiveCounts = {static_cast<uint32_t>(_instances->valueCount())};

    Inherit::compile(context);

    context.buildAccelerationStructureCommands.push_back(BuildAccelerationStructureCommand::create(context.device, _accelerationStructureBuildGeometryInfo, _accelerationStructure, _geometryPrimitiveCounts));
}
