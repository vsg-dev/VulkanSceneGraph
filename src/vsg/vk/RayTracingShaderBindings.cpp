/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/RayTracingShaderBindings.h>

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Extensions.h>
#include <vsg/traversals/CompileTraversal.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////
//
// RayTracingShaderGroup
//
RayTracingShaderBindings::RayTracingShaderBindings()
{
}

RayTracingShaderBindings::RayTracingShaderBindings(const ShaderStages& shaders, Device* device, AllocationCallbacks* allocator) :
    Inherit(nullptr),
    _device(device),
    _shaderStages(shaders),
    _raygenIndex(-1),
    _missIndex(-1),
    _closestHitIndex(-1),
    _callableIndex(-1)
{
    // generate create infos and binding indicies
    for (uint32_t i = 0; i < _shaderStages.size(); i++)
    {

        VkRayTracingShaderGroupCreateInfoNV createinfo;
        createinfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        createinfo.generalShader = VK_SHADER_UNUSED_NV;
        createinfo.closestHitShader = VK_SHADER_UNUSED_NV;
        createinfo.anyHitShader = VK_SHADER_UNUSED_NV;
        createinfo.intersectionShader = VK_SHADER_UNUSED_NV;

        switch(_shaderStages[i]->getShaderStageFlagBits())
        {
            case VK_SHADER_STAGE_RAYGEN_BIT_NV:
                createinfo.generalShader = i;
                _raygenIndex = i;
                break;
            case VK_SHADER_STAGE_MISS_BIT_NV:
                createinfo.generalShader = i;
                _missIndex = i;
                break;
            case VK_SHADER_STAGE_ANY_HIT_BIT_NV:
                createinfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
                createinfo.anyHitShader = i;
                break;
            case VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV:
                createinfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
                createinfo.closestHitShader = i;
                _closestHitIndex = i;
                break;
            case VK_SHADER_STAGE_INTERSECTION_BIT_NV:
                createinfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
                createinfo.intersectionShader = i;
                break;
            case VK_SHADER_STAGE_CALLABLE_BIT_NV:
                createinfo.generalShader = i;
                break;
            default: break;
        }

        _shaderGroupCreateInfos.push_back(createinfo);
    }
}

RayTracingShaderBindings::~RayTracingShaderBindings()
{
}

void RayTracingShaderBindings::read(Input& input)
{
    Object::read(input);
}

void RayTracingShaderBindings::write(Output& output) const
{
    Object::write(output);
}

VkDeviceSize RayTracingShaderBindings::copyShaderIdentifier(uint8_t* data, const uint8_t* shaderHandleStorage, uint32_t groupIndex, uint32_t shaderGroupHandleSize)
{
    memcpy(data, shaderHandleStorage + groupIndex * shaderGroupHandleSize, shaderGroupHandleSize);
    data += shaderGroupHandleSize;
    return shaderGroupHandleSize;
}

void RayTracingShaderBindings::compile(Context& context, const VkPipeline& pipeline)
{
    Extensions* extensions = Extensions::Get(_device, true);

    // query raytracing properties of device
    _rayTracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
    _rayTracingProperties.pNext = nullptr;
    VkPhysicalDeviceProperties2 deviceProps2;
    deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProps2.pNext = &_rayTracingProperties;
    vkGetPhysicalDeviceProperties2(*_device->getPhysicalDevice(), &deviceProps2);

    const uint32_t shaderGroupHandleSize = _rayTracingProperties.shaderGroupHandleSize;
    const uint32_t sbtSize = shaderGroupHandleSize * _shaderGroupCreateInfos.size();

    BufferData bindingTableBufferData = context.stagingMemoryBufferPools.reserveBufferData(sbtSize, 4, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    _bindingTableBuffer = ref_ptr<Buffer>(bindingTableBufferData._buffer);
    _bindingTableMemory = ref_ptr<DeviceMemory>(_bindingTableBuffer->getDeviceMemory());

    if (!_bindingTableMemory)
    {
        return;
    }

    void* buffer_data;
    _bindingTableMemory->map(_bindingTableBuffer->getMemoryOffset() + bindingTableBufferData._offset, bindingTableBufferData._range, 0, &buffer_data);
    uint8_t* ptr = reinterpret_cast<uint8_t*>(buffer_data);

    // get shader handles
    auto shaderHandleStorage = new uint8_t[sbtSize];
    extensions->vkGetRayTracingShaderGroupHandlesNV(*_device, pipeline, 0, _shaderGroupCreateInfos.size(), sbtSize, shaderHandleStorage);

    if(_raygenIndex != -1) ptr += copyShaderIdentifier(ptr, shaderHandleStorage, _raygenIndex, shaderGroupHandleSize);
    if (_missIndex != -1) ptr += copyShaderIdentifier(ptr, shaderHandleStorage, _missIndex, shaderGroupHandleSize);
    if (_closestHitIndex != -1)  ptr += copyShaderIdentifier(ptr, shaderHandleStorage, _closestHitIndex, shaderGroupHandleSize);

    _bindingTableMemory->unmap();
}

VkBuffer RayTracingShaderBindings::raygenTableBuffer() const
{
    return _raygenIndex != -1 ? (VkBuffer)*_bindingTableBuffer : VK_NULL_HANDLE;
}

VkDeviceSize RayTracingShaderBindings::raygeTableOffset() const
{
    return _raygenIndex != -1 ? _rayTracingProperties.shaderGroupHandleSize * _raygenIndex : 0;
}

VkBuffer RayTracingShaderBindings::missTableBuffer() const
{
    return _missIndex != -1 ? (VkBuffer)*_bindingTableBuffer : VK_NULL_HANDLE;
}

VkDeviceSize RayTracingShaderBindings::missTableOffset() const
{
    return _missIndex != -1 ? _rayTracingProperties.shaderGroupHandleSize * _missIndex : 0;
}

VkDeviceSize RayTracingShaderBindings::missTableStride() const
{
    return _missIndex != -1 ? _rayTracingProperties.shaderGroupHandleSize : 0;
}

VkBuffer RayTracingShaderBindings::hitTableBuffer() const
{
    return _closestHitIndex != -1 ? (VkBuffer)*_bindingTableBuffer : VK_NULL_HANDLE;
}

VkDeviceSize RayTracingShaderBindings::hitTableOffset() const
{
    return _closestHitIndex != -1 ? _rayTracingProperties.shaderGroupHandleSize * _closestHitIndex : 0;
}

VkDeviceSize RayTracingShaderBindings::hitTableStride() const
{
    return _closestHitIndex != -1 ? _rayTracingProperties.shaderGroupHandleSize : 0;
}

VkBuffer RayTracingShaderBindings::callableTableBuffer() const
{
    return _callableIndex != -1 ? (VkBuffer)*_bindingTableBuffer : VK_NULL_HANDLE;
}

VkDeviceSize RayTracingShaderBindings::callableTableOffset() const
{
    return _callableIndex != -1 ? _rayTracingProperties.shaderGroupHandleSize * _callableIndex : 0;
}

VkDeviceSize RayTracingShaderBindings::callableTableStride() const
{
    return _callableIndex != -1 ? _rayTracingProperties.shaderGroupHandleSize : 0;
}
