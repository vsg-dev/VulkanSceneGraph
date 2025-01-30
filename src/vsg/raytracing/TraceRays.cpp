/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/raytracing/TraceRays.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

TraceRays::TraceRays()
{
}

void TraceRays::record(CommandBuffer& commandBuffer) const
{
    Device* device = commandBuffer.getDevice();
    auto extensions = device->getExtensions();
    auto rayTracingProperties = device->getPhysicalDevice()->getProperties<VkPhysicalDeviceRayTracingPipelinePropertiesKHR, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR>();
    auto alignedSize = [](uint32_t value, uint32_t alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    };
    uint32_t handleSizeAligned = alignedSize(rayTracingProperties.shaderGroupHandleSize, rayTracingProperties.shaderGroupHandleAlignment);

    auto stridedDeviceAddress = [&](const auto& shaderGroup) {
        if (!shaderGroup) return VkStridedDeviceAddressRegionKHR{};
        VkBufferDeviceAddressInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        info.buffer = shaderGroup->bufferInfo->buffer->vk(device->deviceID);
        if (shaderGroup && shaderGroup->bufferInfo->buffer) return VkStridedDeviceAddressRegionKHR{extensions->vkGetBufferDeviceAddressKHR(*device, &info), handleSizeAligned, shaderGroup->bufferInfo->range};
        return VkStridedDeviceAddressRegionKHR{};
    };

    auto raygenShaderBindingTable = stridedDeviceAddress(raygen);
    auto missShaderBindingTable = stridedDeviceAddress(missShader);
    auto hitShaderBindingTable = stridedDeviceAddress(hitShader);
    auto callableShaderBindingTable = stridedDeviceAddress(callableShader);

    extensions->vkCmdTraceRaysKHR(
        commandBuffer,
        &raygenShaderBindingTable,
        &missShaderBindingTable,
        &hitShaderBindingTable,
        &callableShaderBindingTable,
        width,
        height,
        depth);
}
