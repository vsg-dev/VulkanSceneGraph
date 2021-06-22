#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <map>
#include <vsg/vk/PhysicalDevice.h>

namespace vsg
{

    /// DeviceFeatures is a container class for setting up Vulkan features structures to be passed in vsg::Device creation.
    /// Automatically deletes associated created feature structures on destructions.
    class VSG_DECLSPEC DeviceFeatures : public Inherit<Object, DeviceFeatures>
    {
    public:
        DeviceFeatures();

        DeviceFeatures(const DeviceFeatures&) = delete;
        DeviceFeatures& operator=(const DeviceFeatures&) = delete;

        /// get a Vulkan extension feature structure.
        /// usage example :
        ///     auto& meshFeatures = features->get<VkPhysicalDeviceMeshShaderFeaturesNV, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV>();
        ///     meshFeatures.meshShader = 1;
        ///     meshFeatures.taskShader = 1;
        template<typename FeatureStruct, VkStructureType type>
        FeatureStruct& get()
        {
            if (auto itr = _features.find(type); itr != _features.end()) return *reinterpret_cast<FeatureStruct*>(itr->second);

            FeatureStruct* feature = new FeatureStruct{};

            feature->sType = type;
            feature->pNext = nullptr;

            _features[type] = reinterpret_cast<FeatureHeader*>(feature);

            return *feature;
        }

        /// get the standard VkPhysicalDeviceFeatures structure.
        /// usage example :
        ///     deviceFeatures->get().samplerAnisotropy = VK_TRUE;
        VkPhysicalDeviceFeatures& get();

        /// clear all the feature structures
        void clear();

        /// data() is used as the VkCreateDeviceInfo.pNext setting
        /// automatically chains the pNext pointers of the used feature structures
        /// return nullptr when no features structures have been used.
        void* data() const;

    protected:
        ~DeviceFeatures();

        struct FeatureHeader
        {
            VkStructureType sType;
            void* pNext;
        };

        std::map<VkStructureType, FeatureHeader*> _features;
    };
    VSG_type_name(vsg::DeviceFeatures);

} // namespace vsg
