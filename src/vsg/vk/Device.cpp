/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Device.h>

#include <set>

using namespace vsg;

Device::Device(VkDevice device, PhysicalDevice* physicalDevice, AllocationCallbacks* allocator) :
    _device(device),
    _physicalDevice(physicalDevice),
    _allocator(allocator)
{
}

Device::~Device()
{
    if (_device)
    {
        vkDestroyDevice(_device, _allocator);
    }
}

Device::Result Device::create(PhysicalDevice* physicalDevice, Names& layers, Names& deviceExtensions, AllocationCallbacks* allocator)
{
    if (!physicalDevice)
    {
        return Device::Result("Error: vsg::Device::create(...) failed to create logical device, undefined PhysicalDevice.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    std::set<int> uniqueQueueFamiles;
    if (physicalDevice->getGraphicsFamily() >= 0) uniqueQueueFamiles.insert(physicalDevice->getGraphicsFamily());
    if (physicalDevice->getComputeFamily() >= 0) uniqueQueueFamiles.insert(physicalDevice->getComputeFamily());
    if (physicalDevice->getPresentFamily() >= 0) uniqueQueueFamiles.insert(physicalDevice->getPresentFamily());

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    for (int queueFamily : uniqueQueueFamiles)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
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
