/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <algorithm>

#include <vsg/vk/AccelerationStructure.h>

#include <vsg/vk/Context.h>
#include <vsg/vk/Extensions.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

#define TRANSFER_BUFFERS 0

ref_ptr<Buffer> s_scratchBuffer; //RAYTRACING HACK
ref_ptr<DeviceMemory> s_scratchBufferMemory;

class BuildAccelerationStructureCommand : public Inherit<Command, BuildAccelerationStructureCommand>
{
public:
    BuildAccelerationStructureCommand(Device* device, VkAccelerationStructureInfoNV* info, const VkAccelerationStructureNV& structure, Buffer* instanceBuffer, Allocator* allocator = nullptr) :
        Inherit(allocator),
        _device(device),
        _accelerationStructureInfo(info),
        _accelerationStructure(structure),
        _instanceBuffer(instanceBuffer)
    {
    }

    void compile(Context& context) override {}
    void dispatch(CommandBuffer& commandBuffer) const override
    {
        Extensions* extensions = Extensions::Get(_device, true);

        extensions->vkCmdBuildAccelerationStructureNV(commandBuffer,
                                                      _accelerationStructureInfo,
                                                      _instanceBuffer.valid() ? *_instanceBuffer : (VkBuffer)VK_NULL_HANDLE,
                                                      0,
                                                      VK_FALSE,
                                                      _accelerationStructure,
                                                      VK_NULL_HANDLE,
                                                      *s_scratchBuffer,
                                                      0);

        VkMemoryBarrier memoryBarrier;
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.pNext = nullptr;
        memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
        memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);
    }

    ref_ptr<Device> _device;
    VkAccelerationStructureInfoNV* _accelerationStructureInfo;
    VkAccelerationStructureNV _accelerationStructure;
    ref_ptr<Buffer> _instanceBuffer;

};

AccelerationGeometry::AccelerationGeometry(Allocator* allocator) :
    Inherit(allocator)
{
    _geometry.geometry.triangles.vertexData = VK_NULL_HANDLE;
}

void AccelerationGeometry::compile(Context& context)
{
    if (_arrays.empty()) return; // no data set
    if (_geometry.geometry.triangles.vertexData != VK_NULL_HANDLE) return; // already compiled

    DataList vertexDataList;

    uint32_t vertcount = static_cast<uint32_t>(_arrays[0]->valueCount());
    uint32_t strideSize = 0;
    for (auto& a : _arrays)
    {
        strideSize += static_cast<uint32_t>(a->valueSize());
    }

    auto verts = vsg::ubyteArray::create(vertcount * strideSize);

    uint32_t writepos = 0;
    for (uint32_t i = 0; i < vertcount; i++)
    {
        for (uint32_t a = 0; a < _arrays.size(); a++)
        {
            memcpy(verts->data() + writepos, (uint8_t*)_arrays[a]->dataPointer() + (i * _arrays[a]->valueSize()), _arrays[a]->valueSize());
            writepos += static_cast<uint32_t>(_arrays[a]->valueSize());
        }
    }

    vertexDataList.emplace_back(verts);

    DataList indexDataList;
    indexDataList.emplace_back(_indices);

#if TRANSFER_BUFFERS
    auto vertexBufferData = vsg::createBufferAndTransferData(context, vertexDataList, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    auto indexBufferData = vsg::createBufferAndTransferData(context, indexDataList, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
#else
    auto vertexBufferData = vsg::createHostVisibleBuffer(context.device, vertexDataList, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    auto indexBufferData = vsg::createHostVisibleBuffer(context.device, indexDataList, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
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
    _geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
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

AccelerationStructure::AccelerationStructure(VkAccelerationStructureTypeNV type, Device* device, Allocator* allocator) :
    Inherit(allocator),
    _accelerationStructureInfo({}),
    _device(device)
{
    _accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    _accelerationStructureInfo.type = type;
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
    }
    else
    {
        // error
    }
}
void AccelerationStructure::dispatch(CommandBuffer& commandBuffer) const
{

}

// Bottom Level

BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(Device* device, Allocator* allocator) :
    Inherit(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV, device, allocator)
{
}

void BottomLevelAccelerationStructure::compile(Context& context)
{
    if (_geometries.size() == 0) return; // no data
    if (_vkGeometries.size() == _geometries.size()) return; // already compiled

    for (auto& geom : _geometries)
    {
        geom->compile(context);
        _vkGeometries.push_back(*geom);
    }

    // set the aditional acceleration structure info used in the base AccelerationStructure compile function
    _accelerationStructureInfo.geometryCount = static_cast<uint32_t>(_geometries.size());
    _accelerationStructureInfo.pGeometries = _vkGeometries.data();

    Inherit::compile(context);

    context.commands.push_back(BuildAccelerationStructureCommand::create(context.device, &_accelerationStructureInfo, _accelerationStructure, nullptr));
}

void BottomLevelAccelerationStructure::dispatch(CommandBuffer& commandBuffer) const
{
}

// Top Level

TopLevelAccelerationStructure::TopLevelAccelerationStructure(Device* device, Allocator* allocator) :
    Inherit(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV, device, allocator)
{
}

void TopLevelAccelerationStructure::compile(Context& context)
{
    if (!_instanceSource) return; // no data
    if (_instance) return;        // already compiled

    _instanceSource->compile(context);

    _instance = GeometryInstanceValue::create();
    for (unsigned int i = 0; i < 12; i++)
        _instance->value().transform[i] = (float)_transform.data()[i];
    _instance->value().instanceId = 0;
    _instance->value().mask = 0xff;
    _instance->value().instanceOffset = 0;
    _instance->value().flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
    _instance->value().accelerationStructureHandle = _instanceSource->handle();

    DataList dataList = {_instance};

#if TRANSFER_BUFFERS
    auto instanceBufferData = vsg::createBufferAndTransferData(context, dataList, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_SHARING_MODE_EXCLUSIVE);
    _instanceBuffer = instanceBufferData[0]._buffer;
#else
    auto instanceBufferData = vsg::createHostVisibleBuffer(context.device, dataList, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_SHARING_MODE_EXCLUSIVE);
    _instanceBuffer = instanceBufferData[0]._buffer;
#endif

    _accelerationStructureInfo.instanceCount = 1;

    Inherit::compile(context);

    // now build the accel structures, this probably needs a seperate build function and transverse as we should allocate one
    // scratch buffer the size of the max _requiredBuildScratchSize value then reuse it for all our build calls
    // but in this instance we're just going to make one for this top level and it's referenced bottom level

    const VkDeviceSize scratchBufferSize = std::max(_instanceSource->requiredScratchSize(), requiredScratchSize());
    s_scratchBuffer = Buffer::create(context.device, scratchBufferSize, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_SHARING_MODE_EXCLUSIVE); // RAYTRACING HACK

    s_scratchBufferMemory = vsg::DeviceMemory::create(context.device, s_scratchBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    s_scratchBuffer->bind(s_scratchBufferMemory, 0);

    context.commands.push_back(BuildAccelerationStructureCommand::create(context.device, &_accelerationStructureInfo, _accelerationStructure, _instanceBuffer));
}

void TopLevelAccelerationStructure::dispatch(CommandBuffer& commandBuffer) const
{
}

// DescriptorAccelerationStructure

DescriptorAccelerationStructure::DescriptorAccelerationStructure() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV)
{
}

DescriptorAccelerationStructure::DescriptorAccelerationStructure(const AccelerationStructures& accelerationStructures, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType),
    _accelerationStructures(accelerationStructures)
{
}

void DescriptorAccelerationStructure::read(Input& input)
{
    _accelerationStructures.clear();

    Descriptor::read(input);
}

void DescriptorAccelerationStructure::write(Output& output) const
{
    Descriptor::write(output);
}

void DescriptorAccelerationStructure::compile(Context& context)
{
    // check if we have already compiled the imageData.
    if (_vkAccelerationStructures.size() == _accelerationStructures.size()) return;

    for (const auto& accelstruct : _accelerationStructures)
    {
        accelstruct->compile(context);
        _vkAccelerationStructures.push_back(*accelstruct);
    }

    _descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
    _descriptorAccelerationStructureInfo.accelerationStructureCount = static_cast<uint32_t>(_vkAccelerationStructures.size());
    _descriptorAccelerationStructureInfo.pAccelerationStructures = _vkAccelerationStructures.data();
    _descriptorAccelerationStructureInfo.pNext = nullptr;
}

bool DescriptorAccelerationStructure::assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
{
    Descriptor::assignTo(wds, descriptorSet);

    wds.descriptorCount = static_cast<uint32_t>(_vkAccelerationStructures.size()); // is this correct??
    wds.pNext = &_descriptorAccelerationStructureInfo;

    return true;
}

uint32_t DescriptorAccelerationStructure::getNumDescriptors() const
{
    return static_cast<uint32_t>(_accelerationStructures.size());
}