/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/viewer/Window.h>
#include <vsg/vk/Device.h>

#include <set>

using namespace vsg;

Device::Device(VkDevice device, PhysicalDevice* physicalDevice, AllocationCallbacks* allocator) :
    _device(device),
    _physicalDevice(physicalDevice),
    _allocator(allocator)
{
    // PhysicalDevice only holds a observer_ptr<> to the Instance, so need to take a local reference to the instance to make sure it doesn't get deleted befire we are finsihed with it.
    if (physicalDevice) _instance = physicalDevice->getInstance();
}

Device::~Device()
{
    if (_device)
    {
        vkDestroyDevice(_device, _allocator);
    }
}

Device::Result Device::create(PhysicalDevice* physicalDevice, QueueSettings& queueSettings, Names& layers, Names& deviceExtensions, AllocationCallbacks* allocator)
{
    if (!physicalDevice)
    {
        return Device::Result("Error: vsg::Device::create(...) failed to create logical device, undefined PhysicalDevice.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    for (auto& queueSetting : queueSettings)
    {
        if (queueSetting.queueFamilyIndex < 0) continue;

        // check to see if the queueFamilyIndex has already been referened or us unique
        bool unique = true;
        for (auto& existingInfo : queueCreateInfos)
        {
            if (existingInfo.queueFamilyIndex == static_cast<uint32_t>(queueSetting.queueFamilyIndex)) unique = false;
        }

        // Vylkan doesn't support non unique queueFamily so ignore this entry.
        if (!unique) continue;

        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = static_cast<uint32_t>(queueSetting.queueFamilyIndex);

        if (!queueSetting.queuePiorities.empty())
        {
            queueCreateInfo.queueCount = static_cast<uint32_t>(queueSetting.queuePiorities.size());
            queueCreateInfo.pQueuePriorities = queueSetting.queuePiorities.data();
        }
        else
        {
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
        }

        queueCreateInfo.pNext = nullptr;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.empty() ? nullptr : queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.empty() ? nullptr : deviceExtensions.data();

    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

    createInfo.pNext = nullptr;

    VkDevice device;
    VkResult result = vkCreateDevice(*physicalDevice, &createInfo, allocator, &device);
    if (result == VK_SUCCESS)
    {
        return Result(new Device(device, physicalDevice, allocator));
    }
    else
    {
        return Device::Result("Error: vsg::Device::create(...) failed to create logical device.", result);
    }
}

Device::Result Device::create(WindowTraits* windowTraits)
{
    vsg::Names instanceExtensions = vsg::Window::getInstanceExtensions();

    instanceExtensions.insert(instanceExtensions.end(), windowTraits->instanceExtensionNames.begin(), windowTraits->instanceExtensionNames.end());

    vsg::Names requestedLayers;
    if (windowTraits && windowTraits->debugLayer)
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        requestedLayers.push_back("VK_LAYER_LUNARG_standard_validation");
        if (windowTraits->apiDumpLayer) requestedLayers.push_back("VK_LAYER_LUNARG_api_dump");
    }

    vsg::Names validatedNames = vsg::validateInstancelayerNames(requestedLayers);

    vsg::ref_ptr<vsg::Instance> instance(vsg::Instance::create(instanceExtensions, validatedNames, windowTraits->allocator));
    if (!instance) return Device::Result("Error: vsg::Device::create(...) failed to create logical device.", VK_ERROR_INITIALIZATION_FAILED);

    // set up device
    auto [physicalDevice, queueFamily] = instance->getPhysicalDeviceAndQueueFamily(windowTraits->queueFlags);
    if (!physicalDevice || queueFamily < 0) return Device::Result("Error: vsg::Device::create(...) failed to create logical device.", VK_ERROR_INITIALIZATION_FAILED);

    vsg::Names deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    deviceExtensions.insert(deviceExtensions.end(), windowTraits->deviceExtensionNames.begin(), windowTraits->deviceExtensionNames.end());

    vsg::QueueSettings queueSettings{vsg::QueueSetting{queueFamily, {1.0}}};
    return vsg::Device::create(physicalDevice, queueSettings, validatedNames, deviceExtensions, windowTraits->allocator);
}

ref_ptr<Queue> Device::getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex)
{
    for (auto& queue : _queues)
    {
        if (queue->queueFamilyIndex() == queueFamilyIndex && queue->queueIndex() == queueIndex) return queue;
    }

    VkQueue vk_queue;
    vkGetDeviceQueue(_device, queueFamilyIndex, queueIndex, &vk_queue);

    ref_ptr<Queue> queue(new Queue(vk_queue, queueFamilyIndex, queueIndex));
    _queues.emplace_back(queue);

    return queue;
}
