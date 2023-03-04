#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ref_ptr.h>
#include <vsg/vk/AllocationCallbacks.h>

#include <vector>

namespace vsg
{
    // forward declare
    class PhysicalDevice;
    class Surface;

    using Names = std::vector<const char*>;
    using PhysicalDeviceTypes = std::vector<VkPhysicalDeviceType>;
    using InstanceLayerProperties = std::vector<VkLayerProperties>;
    using InstanceExtensionProperties = std::vector<VkExtensionProperties>;

    /// wrapper for vkEnumerateInstanceExtensionProperties
    extern VSG_DECLSPEC InstanceExtensionProperties enumerateInstanceExtensionProperties(const char* pLayerName = nullptr);

    /// wrapper for vkEnumerateInstanceLayerProperties
    extern VSG_DECLSPEC InstanceLayerProperties enumerateInstanceLayerProperties();

    /// return names of layers that are supported from the desired list.
    extern VSG_DECLSPEC Names validateInstancelayerNames(const Names& names);

    /// Instance encapsulate the vkInstance.
    class VSG_DECLSPEC Instance : public Inherit<Object, Instance>
    {
    public:
        Instance(Names instanceExtensions, Names layers, uint32_t vulkanApiVersion = VK_API_VERSION_1_0, AllocationCallbacks* allocator = nullptr);

        /// Vulkan apiVersion used when creating the VkInstaance
        const uint32_t apiVersion = VK_API_VERSION_1_0;

        operator VkInstance() const { return _instance; }
        VkInstance vk() const { return _instance; }

        AllocationCallbacks* getAllocationCallbacks() { return _allocator.get(); }
        const AllocationCallbacks* getAllocationCallbacks() const { return _allocator.get(); }

        using PhysicalDevices = std::vector<ref_ptr<PhysicalDevice>>;
        PhysicalDevices& getPhysicalDevices() { return _physicalDevices; }
        const PhysicalDevices& getPhysicalDevices() const { return _physicalDevices; }

        /// get a PhysicalDevice that supports the specified queueFlags, and presentation of specified surface if one is provided.
        ref_ptr<PhysicalDevice> getPhysicalDevice(VkQueueFlags queueFlags, const PhysicalDeviceTypes& deviceTypePreferences = {}) const;

        /// get a PhysicalDevice that supports the specified queueFlags, and presentation of specified surface if one is provided.
        ref_ptr<PhysicalDevice> getPhysicalDevice(VkQueueFlags queueFlags, Surface* surface, const PhysicalDeviceTypes& deviceTypePreferences = {}) const;

        /// get a PhysicalDevice and queue family index that supports the specified queueFlags, and presentation of specified surface if one is provided.
        std::pair<ref_ptr<PhysicalDevice>, int> getPhysicalDeviceAndQueueFamily(VkQueueFlags queueFlags, const PhysicalDeviceTypes& deviceTypePreferences = {}) const;

        /// get a PhysicalDevice and queue family index that supports the specified queueFlags, and presentation of specified surface if one is provided.
        std::tuple<ref_ptr<PhysicalDevice>, int, int> getPhysicalDeviceAndQueueFamily(VkQueueFlags queueFlags, Surface* surface, const PhysicalDeviceTypes& deviceTypePreferences = {}) const;

        /// get the address of specified function using vkGetInstanceProcAddr.
        template<typename T>
        bool getProcAddr(T& procAddress, const char* pName, const char* pNameFallback = nullptr) const
        {
            procAddress = reinterpret_cast<T>(vkGetInstanceProcAddr(_instance, pName));
            if (procAddress) return true;

            if (pNameFallback) procAddress = reinterpret_cast<T>(vkGetInstanceProcAddr(_instance, pNameFallback));
            return (procAddress);
        }

    protected:
        virtual ~Instance();

        VkInstance _instance;
        ref_ptr<AllocationCallbacks> _allocator;

        PhysicalDevices _physicalDevices;
    };
    VSG_type_name(vsg::Instance);

} // namespace vsg
