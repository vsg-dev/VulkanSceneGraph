#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/observer_ptr.h>
#include <vsg/vk/Surface.h>

namespace vsg
{
    /// PhysicalDevice encapsulates VkPhysicalDevice
    /// Maps to a Vulkan capable physical device, like a dedicated graphics car or integrated GPU.
    class VSG_DECLSPEC PhysicalDevice : public Inherit<Object, PhysicalDevice>
    {
    public:
        observer_ptr<Instance> getInstance() { return _instance; }

        operator VkPhysicalDevice() const { return _device; }
        VkPhysicalDevice vk() const { return _device; }

        int getQueueFamily(VkQueueFlags queueFlags) const;
        std::pair<int, int> getQueueFamily(VkQueueFlags queueFlags, Surface* surface) const;

        using QueueFamilyProperties = std::vector<VkQueueFamilyProperties>;
        const QueueFamilyProperties& getQueueFamilyProperties() const { return _queueFamiles; }

        const VkPhysicalDeviceFeatures& getFeatures() const { return _features; }
        const VkPhysicalDeviceProperties& getProperties() const { return _properties; }

        template<typename FeatureStruct, VkStructureType type>
        FeatureStruct getFeatures() const
        {
            FeatureStruct features = {};
            features.sType = type;

            if (_vkGetPhysicalDeviceFeatures2)
            {
                VkPhysicalDeviceFeatures2 features2 = {};
                features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                features2.pNext = &features;

                _vkGetPhysicalDeviceFeatures2(_device, &features2);
            }

            return features;
        }

        template<typename PropertiesStruct, VkStructureType type>
        PropertiesStruct getProperties() const
        {
            PropertiesStruct properties = {};
            properties.sType = type;

            if (_vkGetPhysicalDeviceProperties2)
            {
                VkPhysicalDeviceProperties2 properties2 = {};
                properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                properties2.pNext = &properties;

                _vkGetPhysicalDeviceProperties2(_device, &properties2);
            }

            return properties;
        }

        /// Call vkEnumerateDeviceExtensionProperties to enumerate extension properties.
        std::vector<VkExtensionProperties> enumerateDeviceExtensionProperties(const char* pLayerName = nullptr);

    protected:
        // use Instance::getDevice(..) to create PhysicalDevice
        PhysicalDevice(Instance* instance, VkPhysicalDevice device);

        virtual ~PhysicalDevice();

        friend class Instance;

        VkPhysicalDevice _device;

        VkPhysicalDeviceFeatures _features;
        VkPhysicalDeviceProperties _properties;
        QueueFamilyProperties _queueFamiles;

        PFN_vkGetPhysicalDeviceFeatures2 _vkGetPhysicalDeviceFeatures2 = nullptr;
        PFN_vkGetPhysicalDeviceProperties2 _vkGetPhysicalDeviceProperties2 = nullptr;

        vsg::observer_ptr<Instance> _instance;
    };
    VSG_type_name(vsg::PhysicalDevice);

} // namespace vsg
