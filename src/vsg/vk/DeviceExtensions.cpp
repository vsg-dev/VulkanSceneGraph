/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/vk/Device.h>

#include <algorithm>
#include <cstring>

using namespace vsg;

DeviceExtensions::DeviceExtensions(Device* device)
{
    // VK_EXT_host_query_reset
    device->getProcAddr(vkResetQueryPool, "vkResetQueryPool", "vkResetQueryPoolEXT");

    // VK_KHR_create_renderpass2
    if (device->supportsApiVersion(VK_API_VERSION_1_2))
        device->getProcAddr(vkCreateRenderPass2, "vkCreateRenderPass2");
    else if (device->getPhysicalDevice()->supportsDeviceExtension(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME))
        device->getProcAddr(vkCreateRenderPass2, "vkCreateRenderPass2KHR");

    // VK_KHR_ray_tracing
    device->getProcAddr(vkCreateAccelerationStructureKHR, "vkCreateAccelerationStructureKHR");
    device->getProcAddr(vkDestroyAccelerationStructureKHR, "vkDestroyAccelerationStructureKHR");
    device->getProcAddr(vkGetAccelerationStructureDeviceAddressKHR, "vkGetAccelerationStructureDeviceAddressKHR");
    device->getProcAddr(vkGetAccelerationStructureBuildSizesKHR, "vkGetAccelerationStructureBuildSizesKHR");
    device->getProcAddr(vkCmdBuildAccelerationStructuresKHR, "vkCmdBuildAccelerationStructuresKHR");
    device->getProcAddr(vkCreateRayTracingPipelinesKHR, "vkCreateRayTracingPipelinesKHR");
    device->getProcAddr(vkGetRayTracingShaderGroupHandlesKHR, "vkGetRayTracingShaderGroupHandlesKHR");
    device->getProcAddr(vkCmdTraceRaysKHR, "vkCmdTraceRaysKHR");

    device->getProcAddr(vkGetBufferDeviceAddressKHR, "vkGetBufferDeviceAddressKHR");

    // VK_EXT_mesh_shader
    device->getProcAddr(vkCmdDrawMeshTasksEXT, "vkCmdDrawMeshTasksEXT");
    device->getProcAddr(vkCmdDrawMeshTasksIndirectEXT, "vkCmdDrawMeshTasksIndirectEXT");
    device->getProcAddr(vkCmdDrawMeshTasksIndirectCountEXT, "vkCmdDrawMeshTasksIndirectCountEXT");

    // VK_EXT_extended_dynamic_state
    if (device->supportsApiVersion(VK_API_VERSION_1_3))
    {
        device->getProcAddr(vkCmdSetCullMode, "vkCmdSetCullMode");
        device->getProcAddr(vkCmdSetFrontFace, "vkCmdSetFrontFace");
        device->getProcAddr(vkCmdSetPrimitiveTopology, "vkCmdSetPrimitiveTopology");
        device->getProcAddr(vkCmdSetViewportWithCount, "vkCmdSetViewportWithCount");
        device->getProcAddr(vkCmdSetScissorWithCount, "vkCmdSetScissorWithCount");
        device->getProcAddr(vkCmdBindVertexBuffers2, "vkCmdBindVertexBuffers2");
        device->getProcAddr(vkCmdSetDepthTestEnable, "vkCmdSetDepthTestEnable");
        device->getProcAddr(vkCmdSetDepthWriteEnable, "vkCmdSetDepthWriteEnable");
        device->getProcAddr(vkCmdSetDepthCompareOp, "vkCmdSetDepthCompareOp");
        device->getProcAddr(vkCmdSetDepthBoundsTestEnable, "vkCmdSetDepthBoundsTestEnable");
        device->getProcAddr(vkCmdSetStencilTestEnable, "vkCmdSetStencilTestEnable");
        device->getProcAddr(vkCmdSetStencilOp, "vkCmdSetStencilOp");
    }
    else if (device->supportsDeviceExtension("VK_EXT_extended_dynamic_state"))
    {
        device->getProcAddr(vkCmdSetCullMode, "vkCmdSetCullModeEXT");
        device->getProcAddr(vkCmdSetFrontFace, "vkCmdSetFrontFaceEXT");
        device->getProcAddr(vkCmdSetPrimitiveTopology, "vkCmdSetPrimitiveTopologyEXT");
        device->getProcAddr(vkCmdSetViewportWithCount, "vkCmdSetViewportWithCountEXT");
        device->getProcAddr(vkCmdSetScissorWithCount, "vkCmdSetScissorWithCountEXT");
        device->getProcAddr(vkCmdBindVertexBuffers2, "vkCmdBindVertexBuffers2EXT");
        device->getProcAddr(vkCmdSetDepthTestEnable, "vkCmdSetDepthTestEnableEXT");
        device->getProcAddr(vkCmdSetDepthWriteEnable, "vkCmdSetDepthWriteEnableEXT");
        device->getProcAddr(vkCmdSetDepthCompareOp, "vkCmdSetDepthCompareOpEXT");
        device->getProcAddr(vkCmdSetDepthBoundsTestEnable, "vkCmdSetDepthBoundsTestEnableEXT");
        device->getProcAddr(vkCmdSetStencilTestEnable, "vkCmdSetStencilTestEnableEXT");
        device->getProcAddr(vkCmdSetStencilOp, "vkCmdSetStencilOpEXT");
    }
}
