/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Options.h>
#include <vsg/vk/Instance.h>
#include <vsg/vk/PhysicalDevice.h>

#include <set>

using namespace vsg;

Names vsg::validateInstancelayerNames(const Names& names)
{
    if (names.empty()) return names;

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    using NameSet = std::set<std::string>;
    NameSet layerNames;
    for (const auto& layer : availableLayers)
    {
        //std::cout << "layer=" << layer.layerName << std::endl;
        if (layer.layerName[0] != 0) layerNames.insert(layer.layerName);
    }

    Names validatedNames;
    validatedNames.reserve(names.size());
    for (const auto& requestedName : names)
    {
        if (layerNames.count(requestedName) != 0)
        {
            //std::cout << "valid requested layer : " << requestedName << std::endl;
            validatedNames.push_back(requestedName);
        }
        else
        {
            //std::cout << "Warning : requested invalid layer : " << requestedName << std::endl;
        }
    }

    return validatedNames;
}

Instance::Instance(const Names& instanceExtensions, const Names& layers, uint32_t vulkanApiVersion, AllocationCallbacks* allocator)
{
    // application info
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanSceneGraph application";
    appInfo.pEngineName = "VulkanSceneGraph";
    appInfo.engineVersion = VK_MAKE_VERSION(VSG_VERSION_MAJOR, VSG_VERSION_MINOR, VSG_VERSION_PATCH);
    appInfo.apiVersion = vulkanApiVersion;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.empty() ? nullptr : instanceExtensions.data();

    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

    createInfo.pNext = nullptr;

    VkInstance instance;
    VkResult result = vkCreateInstance(&createInfo, allocator, &instance);
    if (result == VK_SUCCESS)
    {
        apiVersion = vulkanApiVersion;

        _instance = instance;
        _allocator = allocator;

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (auto device : devices)
        {
            _physicalDevices.emplace_back(new PhysicalDevice(this, device));
        }
    }
    else
    {
        throw Exception{"Error: vsg::Instance::create(...) failed to create VkInstance.", result};
    }
}

Instance::~Instance()
{
    _physicalDevices.clear();

    if (_instance)
    {
        vkDestroyInstance(_instance, _allocator);
    }
}

ref_ptr<PhysicalDevice> Instance::getPhysicalDevice(VkQueueFlags queueFlags, const PhysicalDeviceTypes& deviceTypePreferences) const
{
    for (size_t i = 0; i <= deviceTypePreferences.size(); ++i)
    {
        for (auto& device : _physicalDevices)
        {
            if (i < deviceTypePreferences.size() && device->getProperties().deviceType != deviceTypePreferences[i]) continue;

            if (auto family = device->getQueueFamily(queueFlags); (family >= 0)) return device;
        }
    }
    return {};
}

ref_ptr<PhysicalDevice> Instance::getPhysicalDevice(VkQueueFlags queueFlags, Surface* surface, const PhysicalDeviceTypes& deviceTypePreferences) const
{
    for (size_t i = 0; i <= deviceTypePreferences.size(); ++i)
    {
        for (auto& device : _physicalDevices)
        {
            if (i < deviceTypePreferences.size() && device->getProperties().deviceType != deviceTypePreferences[i]) continue;

            if (auto [graphicsFamily, presentFamily] = device->getQueueFamily(queueFlags, surface); (graphicsFamily >= 0 && presentFamily >= 0)) return device;
        }
    }
    return {};
}

std::pair<ref_ptr<PhysicalDevice>, int> Instance::getPhysicalDeviceAndQueueFamily(VkQueueFlags queueFlags, const PhysicalDeviceTypes& deviceTypePreferences) const
{
    for (size_t i = 0; i <= deviceTypePreferences.size(); ++i)
    {
        for (auto& device : _physicalDevices)
        {
            if (i < deviceTypePreferences.size() && device->getProperties().deviceType != deviceTypePreferences[i]) continue;

            if (auto family = device->getQueueFamily(queueFlags); family >= 0) return {device, family};
        }
    }
    return {{}, -1};
}

std::tuple<ref_ptr<PhysicalDevice>, int, int> Instance::getPhysicalDeviceAndQueueFamily(VkQueueFlags queueFlags, Surface* surface, const PhysicalDeviceTypes& deviceTypePreferences) const
{
    for (size_t i = 0; i <= deviceTypePreferences.size(); ++i)
    {
        for (auto& device : _physicalDevices)
        {
            if (i < deviceTypePreferences.size() && device->getProperties().deviceType != deviceTypePreferences[i]) continue;

            if (auto [graphicsFamily, presentFamily] = device->getQueueFamily(queueFlags, surface); (graphicsFamily >= 0 && presentFamily >= 0))
            {
                return {device, graphicsFamily, presentFamily};
            }
        }
    }
    return {{}, -1, -1};
}
