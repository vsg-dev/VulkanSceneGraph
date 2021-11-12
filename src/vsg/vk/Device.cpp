/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/Version.h>
#include <vsg/io/Options.h>
#include <vsg/viewer/Window.h>
#include <vsg/vk/Device.h>

#include <cstring>
#include <iostream>
#include <set>
using namespace vsg;

// thread safe container for managing the deviceID for each vsg;:Device
static std::mutex s_DeviceCountMutex;
static std::vector<bool> s_ActiveDevices;

static uint32_t getUniqueDeviceID()
{
    std::scoped_lock<std::mutex> guard(s_DeviceCountMutex);

    uint32_t deviceID = 0;
    for (deviceID = 0; deviceID < static_cast<uint32_t>(s_ActiveDevices.size()); ++deviceID)
    {
        if (!s_ActiveDevices[deviceID])
        {
            s_ActiveDevices[deviceID] = true;
            return deviceID;
        }
    }

    s_ActiveDevices.push_back(true);

    return deviceID;
}

static void releaseDeviceID(uint32_t deviceID)
{
    std::scoped_lock<std::mutex> guard(s_DeviceCountMutex);
    s_ActiveDevices[deviceID] = false;
}

Device::Device(PhysicalDevice* physicalDevice, const QueueSettings& queueSettings, const Names& layers, const Names& deviceExtensions, const DeviceFeatures* deviceFeatures, AllocationCallbacks* allocator) :
    deviceID(getUniqueDeviceID()),
    _instance(physicalDevice->getInstance()),
    _physicalDevice(physicalDevice),
    _allocator(allocator)
{
    if (deviceID >= VSG_MAX_DEVICES)
    {
        releaseDeviceID(deviceID);
        throw Exception{"Warning : number of vsg:Device allocated exceeds number supported ", VSG_MAX_DEVICES};
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    for (auto& queueSetting : queueSettings)
    {
        if (queueSetting.queueFamilyIndex < 0) continue;

        // check to see if the queueFamilyIndex has already been referenced or us unique
        bool unique = true;
        for (auto& existingInfo : queueCreateInfos)
        {
            if (existingInfo.queueFamilyIndex == static_cast<uint32_t>(queueSetting.queueFamilyIndex)) unique = false;
        }

        // Vulkan doesn't support non unique queueFamily so ignore this entry.
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

    // MacOS requires "VK_KHR_portability_subset" to be a requested extension if the PhysicalDevice supported it.
    Names local_deviceExtensions(deviceExtensions);
    auto extensionProperties = _physicalDevice->enumerateDeviceExtensionProperties();
    for (auto& extensionProperty : extensionProperties)
    {
        if (std::strncmp(extensionProperty.extensionName, "VK_KHR_portability_subset", VK_MAX_EXTENSION_NAME_SIZE) == 0)
        {
            local_deviceExtensions.push_back(extensionProperty.extensionName);
        }
    }

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.empty() ? nullptr : queueCreateInfos.data();

    createInfo.pEnabledFeatures = nullptr;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(local_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = local_deviceExtensions.empty() ? nullptr : local_deviceExtensions.data();

    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

    createInfo.pNext = deviceFeatures ? deviceFeatures->data() : nullptr;

    VkResult result = vkCreateDevice(*physicalDevice, &createInfo, allocator, &_device);
    if (result != VK_SUCCESS)
    {
        releaseDeviceID(deviceID);
        throw Exception{"Error: vsg::Device::create(...) failed to create logical device.", result};
    }
}

Device::~Device()
{
    if (_device)
    {
        vkDestroyDevice(_device, _allocator);
    }

    releaseDeviceID(deviceID);
}

uint32_t Device::maxNumDevices()
{
    return VSG_MAX_DEVICES;
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
