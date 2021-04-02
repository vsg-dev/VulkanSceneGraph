#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Device.h>
#include <vsg/vk/Instance.h>

namespace vsg
{
    using ExtensionProperties = std::vector<VkExtensionProperties>;

    extern VSG_DECLSPEC ExtensionProperties getExtensionProperties(const char* pLayerName = nullptr);

    extern VSG_DECLSPEC bool isExtensionSupported(const char* extensionName);

    extern VSG_DECLSPEC bool isExtensionListSupported(const Names& extensionList);

    // TODO need to reorganize so that the Device "has a" extension structure and avoid the usage of static container
    class VSG_DECLSPEC Extensions : public Object
    {
    public:
        static Extensions* Get(Device* device, bool createIfNotInitalized);

        Extensions(Device* device);

        // VK_NV_ray_tracing
        PFN_vkCreateAccelerationStructureNV vkCreateAccelerationStructureNV = nullptr;
        PFN_vkDestroyAccelerationStructureNV vkDestroyAccelerationStructureNV = nullptr;
        PFN_vkBindAccelerationStructureMemoryNV vkBindAccelerationStructureMemoryNV = nullptr;
        PFN_vkGetAccelerationStructureHandleNV vkGetAccelerationStructureHandleNV = nullptr;
        PFN_vkGetAccelerationStructureMemoryRequirementsNV vkGetAccelerationStructureMemoryRequirementsNV = nullptr;
        PFN_vkCmdBuildAccelerationStructureNV vkCmdBuildAccelerationStructureNV = nullptr;
        PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV = nullptr;
        PFN_vkGetRayTracingShaderGroupHandlesNV vkGetRayTracingShaderGroupHandlesNV = nullptr;
        PFN_vkCmdTraceRaysNV vkCmdTraceRaysNV = nullptr;

        // VK_NV_mesh_shader
        PFN_vkCmdDrawMeshTasksNV vkCmdDrawMeshTasksNV = nullptr;
    };

} // namespace vsg
