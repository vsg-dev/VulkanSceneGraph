/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <algorithm>

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/raytracing/AccelerationGeometry.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>

#define TRANSFER_BUFFERS 0

using namespace vsg;

AccelerationGeometry::AccelerationGeometry() :
    _geometry({})
{
    _geometry.geometry.triangles.vertexData.deviceAddress = VkDeviceAddress{0};
}

void AccelerationGeometry::assignVertices(ref_ptr<vsg::Data> in_vertices)
{
    verts = in_vertices;
}

void AccelerationGeometry::assignIndices(ref_ptr<vsg::Data> in_indices)
{
    indices = in_indices;
}

void AccelerationGeometry::compile(Context& context)
{
    if (!verts) return;                                                                      // no data set
    if (_geometry.geometry.triangles.vertexData.deviceAddress != VkDeviceAddress{0}) return; // already compiled

    uint32_t vertcount = static_cast<uint32_t>(verts->valueCount());
    uint32_t strideSize = static_cast<uint32_t>(verts->valueSize());

    DataList vertexDataList;
    vertexDataList.push_back(verts);

    DataList indexDataList;
    indexDataList.push_back(indices);

#if TRANSFER_BUFFERS
    auto vertexBufferInfo = vsg::createBufferAndTransferData(context, vertexDataList, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    auto indexBufferInfo = vsg::createBufferAndTransferData(context, indexDataList, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
#else
    auto vertexBufferInfo = vsg::createHostVisibleBuffer(context.device, vertexDataList, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_SHARING_MODE_EXCLUSIVE);
    vsg::copyDataListToBuffers(context.device, vertexBufferInfo);
    auto indexBufferInfo = vsg::createHostVisibleBuffer(context.device, indexDataList, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_SHARING_MODE_EXCLUSIVE);
    vsg::copyDataListToBuffers(context.device, indexBufferInfo);
#endif

    _vertexBuffer = vertexBufferInfo[0];
    _indexBuffer = indexBufferInfo[0];

    // create the VkGeometry
    auto extensions = context.device->getExtensions();
    VkDeviceOrHostAddressConstKHR vertexDataDeviceAddress{};
    VkDeviceOrHostAddressConstKHR indexDataDeviceAddress{};
    VkBufferDeviceAddressInfo bufferDeviceAI{};
    bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAI.buffer = _vertexBuffer->buffer->vk(context.deviceID);
    vertexDataDeviceAddress.deviceAddress = extensions->vkGetBufferDeviceAddressKHR(*context.device, &bufferDeviceAI);
    bufferDeviceAI.buffer = _indexBuffer->buffer->vk(context.deviceID);
    indexDataDeviceAddress.deviceAddress = extensions->vkGetBufferDeviceAddressKHR(*context.device, &bufferDeviceAI);

    _geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    _geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    _geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    _geometry.geometry.triangles.vertexData = vertexDataDeviceAddress;
    _geometry.geometry.triangles.maxVertex = vertcount;
    _geometry.geometry.triangles.vertexStride = strideSize;
    _geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    _geometry.geometry.triangles.indexData = indexDataDeviceAddress;
    _geometry.geometry.triangles.indexType = computeIndexType(indices);
    _geometry.geometry.triangles.pNext = nullptr;
    _geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    _geometry.pNext = nullptr;
}
