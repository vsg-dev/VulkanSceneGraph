/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <algorithm>

#include <vsg/raytracing/AccelerationGeometry.h>

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/Extensions.h>

#define TRANSFER_BUFFERS 0

using namespace vsg;

AccelerationGeometry::AccelerationGeometry(Allocator* allocator) :
    Inherit(allocator)
{
    _geometry.geometry.triangles.vertexData = VK_NULL_HANDLE;
}

void AccelerationGeometry::compile(Context& context)
{
    if (!_verts) return;                                                   // no data set
    if (_geometry.geometry.triangles.vertexData != VK_NULL_HANDLE) return; // already compiled

    uint32_t vertcount = static_cast<uint32_t>(_verts->valueCount());
    uint32_t strideSize = static_cast<uint32_t>(_verts->valueSize());

    DataList vertexDataList;
    vertexDataList.push_back(_verts);

    DataList indexDataList;
    indexDataList.push_back(_indices);

#if TRANSFER_BUFFERS
    auto vertexBufferData = vsg::createBufferAndTransferData(context, vertexDataList, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    auto indexBufferData = vsg::createBufferAndTransferData(context, indexDataList, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
#else
    auto vertexBufferData = vsg::createHostVisibleBuffer(context.device, vertexDataList, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    vsg::copyDataListToBuffers(vertexBufferData);
    auto indexBufferData = vsg::createHostVisibleBuffer(context.device, indexDataList, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    vsg::copyDataListToBuffers(indexBufferData);
#endif

    _vertexBuffer = vertexBufferData[0];
    _indexBuffer = indexBufferData[0];

    // create the VkGeometry
    _geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
    _geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
    _geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
    _geometry.geometry.triangles.vertexData = *_vertexBuffer._buffer;
    _geometry.geometry.triangles.vertexOffset = 0;
    _geometry.geometry.triangles.vertexCount = vertcount;
    _geometry.geometry.triangles.vertexStride = strideSize;
    _geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    _geometry.geometry.triangles.indexData = *_indexBuffer._buffer;
    _geometry.geometry.triangles.indexOffset = 0;
    _geometry.geometry.triangles.indexCount = static_cast<uint32_t>(_indices->valueCount());
    _geometry.geometry.triangles.indexType = _indices->valueSize() > 2 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
    _geometry.geometry.triangles.transformData = VK_NULL_HANDLE;
    _geometry.geometry.triangles.transformOffset = 0;
    _geometry.geometry.triangles.pNext = nullptr;
    _geometry.geometry.aabbs.numAABBs = 0;
    _geometry.geometry.aabbs.aabbData = VK_NULL_HANDLE;
    _geometry.geometry.aabbs.offset = 0;
    _geometry.geometry.aabbs.stride = 0;
    _geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
    _geometry.geometry.aabbs.pNext = nullptr;
    _geometry.flags = VK_GEOMETRY_OPAQUE_BIT_NV;
    _geometry.pNext = nullptr;
}