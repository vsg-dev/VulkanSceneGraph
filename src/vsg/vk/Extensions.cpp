/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Extensions.h>
#include <vsg/io/Options.h>

#include <algorithm>
#include <cstring>

#include <iostream>

using namespace vsg;

ExtensionProperties vsg::getExtensionProperties(const char* pLayerName)
{
    uint32_t extCount = 0;
    VkResult err = vkEnumerateInstanceExtensionProperties(pLayerName, &extCount, nullptr);
    if (err)
    {
        std::cout << "Error: vsg::getExtensionPropertiesCount(...) failed, could not get extension count from vkEnumerateInstanceExtensionProperties." << std::endl;
        return ExtensionProperties();
    }

    ExtensionProperties extensionProperties(extCount);
    err = vkEnumerateInstanceExtensionProperties(pLayerName, &extCount, extensionProperties.data());
    if (err)
    {
        std::cout << "Error: vsg::getExtensionProperties(...) failed, could not get extension properties from vkEnumerateInstanceExtensionProperties." << std::endl;
        return ExtensionProperties();
    }
    return extensionProperties;
}

bool vsg::isExtensionSupported(const char* extensionName)
{
    ExtensionProperties extProps = getExtensionProperties();
    for (auto prop : extProps)
    {
        if (strcmp(prop.extensionName, extensionName) == 0) return true;
    }
    return false;
}

bool vsg::isExtensionListSupported(const Names& extensionList)
{
    ExtensionProperties extProps = getExtensionProperties();
    for (auto ext : extensionList)
    {
        auto compare = [&](const VkExtensionProperties& rhs) { return strcmp(ext, rhs.extensionName) == 0; };
        if (std::find_if(extProps.begin(), extProps.end(), compare) == extProps.end()) return false;
    }
    return true;
}

typedef std::map<Device*, ref_ptr<Extensions>> BufferedExtensions;
static BufferedExtensions s_extensions;

Extensions* Extensions::Get(Device* device, bool createIfNotInitalized)
{
    if (!s_extensions[device] && createIfNotInitalized)
        s_extensions[device] = new Extensions(device);

    return s_extensions[device].get();
}

Extensions::Extensions(Device* device)
{
    // VK_NV_ray_tracing
    vkCreateAccelerationStructureNV = reinterpret_cast<PFN_vkCreateAccelerationStructureNV>(vkGetDeviceProcAddr(*device, "vkCreateAccelerationStructureNV"));
    vkDestroyAccelerationStructureNV = reinterpret_cast<PFN_vkDestroyAccelerationStructureNV>(vkGetDeviceProcAddr(*device, "vkDestroyAccelerationStructureNV"));
    vkBindAccelerationStructureMemoryNV = reinterpret_cast<PFN_vkBindAccelerationStructureMemoryNV>(vkGetDeviceProcAddr(*device, "vkBindAccelerationStructureMemoryNV"));
    vkGetAccelerationStructureHandleNV = reinterpret_cast<PFN_vkGetAccelerationStructureHandleNV>(vkGetDeviceProcAddr(*device, "vkGetAccelerationStructureHandleNV"));
    vkGetAccelerationStructureMemoryRequirementsNV = reinterpret_cast<PFN_vkGetAccelerationStructureMemoryRequirementsNV>(vkGetDeviceProcAddr(*device, "vkGetAccelerationStructureMemoryRequirementsNV"));
    vkCmdBuildAccelerationStructureNV = reinterpret_cast<PFN_vkCmdBuildAccelerationStructureNV>(vkGetDeviceProcAddr(*device, "vkCmdBuildAccelerationStructureNV"));
    vkCreateRayTracingPipelinesNV = reinterpret_cast<PFN_vkCreateRayTracingPipelinesNV>(vkGetDeviceProcAddr(*device, "vkCreateRayTracingPipelinesNV"));
    vkGetRayTracingShaderGroupHandlesNV = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesNV>(vkGetDeviceProcAddr(*device, "vkGetRayTracingShaderGroupHandlesNV"));
    vkCmdTraceRaysNV = reinterpret_cast<PFN_vkCmdTraceRaysNV>(vkGetDeviceProcAddr(*device, "vkCmdTraceRaysNV"));
}
