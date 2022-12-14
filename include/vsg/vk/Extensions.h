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

    extern VSG_DECLSPEC bool isExtensionSupported(const char* extensionName);

    extern VSG_DECLSPEC bool isExtensionListSupported(const Names& extensionList);

    /// Extensions manages a set of Vulkan extension function pointers.
    /// The vsg::Device "has a" Extensions object that can be accessed via device->getExtensions().
    class VSG_DECLSPEC Extensions : public Inherit<Object, Extensions>
    {
    public:
        explicit Extensions(Device* device);

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

        // VK_NV_mesh_shader
        PFN_vkCmdDrawMeshTasksNV vkCmdDrawMeshTasksNV = nullptr;
        PFN_vkCmdDrawMeshTasksIndirectNV vkCmdDrawMeshTasksIndirectNV = nullptr;
        PFN_vkCmdDrawMeshTasksIndirectCountNV vkCmdDrawMeshTasksIndirectCountNV = nullptr;
    };

} // namespace vsg
