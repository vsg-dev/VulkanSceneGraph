#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Instance.h>

namespace vsg
{
    class Device;

    /// Extensions manages a set of Vulkan extension function pointers.
    /// The vsg::Device "has a" DeviceExtensions object that can be accessed via device->getExtensions().
    class VSG_DECLSPEC DeviceExtensions : public Inherit<Object, DeviceExtensions>
    {
    public:
        explicit DeviceExtensions(Device* device);

        // VK_EXT_host_query_reset / Vulkan-1.2
        PFN_vkResetQueryPoolEXT vkResetQueryPool = nullptr;

        // VK_KHR_create_renderpass2
        PFN_vkCreateRenderPass2KHR_Compatibility vkCreateRenderPass2 = nullptr;

        // VK_KHR_ray_tracing
        PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = nullptr;
        PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR = nullptr;
        PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR = nullptr;
        PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = nullptr;
        PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = nullptr;
        PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = nullptr;
        PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR = nullptr;
        PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = nullptr;

        PFN_vkGetBufferDeviceAddressKHR_Compatibility vkGetBufferDeviceAddressKHR = nullptr;

        // VK_EXT_mesh_shader
        PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT = nullptr;
        PFN_vkCmdDrawMeshTasksIndirectEXT vkCmdDrawMeshTasksIndirectEXT = nullptr;
        PFN_vkCmdDrawMeshTasksIndirectCountEXT vkCmdDrawMeshTasksIndirectCountEXT = nullptr;

        // VK_EXT_extended_dynamic_state / Vulkan 1.3
        PFN_vkCmdSetCullMode vkCmdSetCullMode = nullptr;
        PFN_vkCmdSetFrontFace vkCmdSetFrontFace = nullptr;
        PFN_vkCmdSetPrimitiveTopology vkCmdSetPrimitiveTopology = nullptr;
        PFN_vkCmdSetViewportWithCount vkCmdSetViewportWithCount = nullptr;
        PFN_vkCmdSetScissorWithCount vkCmdSetScissorWithCount = nullptr;
        PFN_vkCmdBindVertexBuffers2 vkCmdBindVertexBuffers2 = nullptr;
        PFN_vkCmdSetDepthTestEnable vkCmdSetDepthTestEnable = nullptr;
        PFN_vkCmdSetDepthWriteEnable vkCmdSetDepthWriteEnable = nullptr;
        PFN_vkCmdSetDepthCompareOp vkCmdSetDepthCompareOp = nullptr;
        PFN_vkCmdSetDepthBoundsTestEnable vkCmdSetDepthBoundsTestEnable = nullptr;
        PFN_vkCmdSetStencilTestEnable vkCmdSetStencilTestEnable = nullptr;
        PFN_vkCmdSetStencilOp vkCmdSetStencilOp = nullptr;
    };
    VSG_type_name(vsg::DeviceExtensions);

} // namespace vsg
