/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/PhysicalDevice.h>

using namespace vsg;

PhysicalDevice::PhysicalDevice(Instance* instance, VkPhysicalDevice device) :
    _device(device),
    _instance(instance)
{
    vkGetPhysicalDeviceProperties(_device, &_properties);

    _rayTracingProperties = getProperties<VkPhysicalDeviceRayTracingPropertiesNV, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV>();

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, nullptr);

    _queueFamiles.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, _queueFamiles.data());
#if 0
    std::cout << "shaderGroupHandleSize " << _rayTracingProperties.shaderGroupHandleSize << std::endl;
    std::cout << "maxRecursionDepth " << _rayTracingProperties.maxRecursionDepth << std::endl;
    std::cout << "maxShaderGroupStride " << _rayTracingProperties.maxShaderGroupStride << std::endl;
    std::cout << "shaderGroupBaseAlignment " << _rayTracingProperties.shaderGroupBaseAlignment << std::endl;
    std::cout << "maxGeometryCount " << _rayTracingProperties.maxGeometryCount << std::endl;
    std::cout << "maxInstanceCount " << _rayTracingProperties.maxInstanceCount << std::endl;
    std::cout << "maxTriangleCount " << _rayTracingProperties.maxTriangleCount << std::endl;
    std::cout << "maxDescriptorSetAccelerationStructures " << _rayTracingProperties.maxDescriptorSetAccelerationStructures << std::endl;
#endif
}

PhysicalDevice::~PhysicalDevice()
{
}

int PhysicalDevice::getQueueFamily(VkQueueFlags queueFlags) const
{
    int bestFamily = -1;

    for(int i = 0; i<static_cast<int>(_queueFamiles.size()); ++i)
    {
        const auto& queueFamily = _queueFamiles[i];
        if ((queueFamily.queueFlags & queueFlags) == queueFlags)
        {
            // check for perfect match
            if (queueFamily.queueFlags == queueFlags)
            {
                return i;
            }

            bestFamily = i;
        }
    }

    return bestFamily;
}

std::pair<int, int> PhysicalDevice::getQueueFamily(VkQueueFlags queueFlags, Surface* surface) const
{
    int bestFamily = -1;

    for(int i = 0; i<static_cast<int>(_queueFamiles.size()); ++i)
    {
        const auto& queueFamily = _queueFamiles[i];
        if ((queueFamily.queueFlags & queueFlags) == queueFlags)
        {
            if (surface)
            {
                VkBool32 presentSupported = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(_device, i, *surface, &presentSupported);
                if (presentSupported)
                {
                    // check for perfect match
                    if (queueFamily.queueFlags == queueFlags)
                    {
                        return {i, i};
                    }

                    bestFamily = i;
                }
            }
            else
            {
                // check for perfect match
                if (queueFamily.queueFlags == queueFlags)
                {
                    return {i, i};
                }

                bestFamily = i;
            }
        }
    }

    return {bestFamily, bestFamily};
}
