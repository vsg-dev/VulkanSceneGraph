#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/vk_buffer.h>

namespace vsg
{
    // forward declare
    class Context;

    class VSG_DECLSPEC Image : public Inherit<Object, Image>
    {
    public:
        /// create a vsg::Image with optional CreateInfo, delay VkUmage creation to compile
        Image(ref_ptr<Data> in_data = {});

        /// create a vsg::Image wrapper for specified VkImage
        Image(VkImage image, Device* device);

        /// VkImageCreateInfo settings
        ref_ptr<Data> data;
        VkImageCreateFlags flags = 0;
        VkImageType imageType = VK_IMAGE_TYPE_2D;
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkExtent3D extent = {0, 0, 0};
        uint32_t mipLevels = 0;
        uint32_t arrayLayers = 0;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
        VkImageUsageFlags usage = 0;
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        std::vector<uint32_t> queueFamilyIndices;
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        /// Vulkan VkImage handle
        VkImage vk(uint32_t deviceID) const { return _vulkanData[deviceID].image; }

        DeviceMemory* getDeviceMemory(uint32_t deviceID) { return _vulkanData[deviceID].deviceMemory; }
        const DeviceMemory* getDeviceMemory(uint32_t deviceID) const { return _vulkanData[deviceID].deviceMemory; }

        VkDeviceSize getMemoryOffset(uint32_t deviceID) const { return _vulkanData[deviceID].memoryOffset; }

        VkMemoryRequirements getMemoryRequirements(uint32_t deviceID) const;

        VkResult bind(DeviceMemory* deviceMemory, VkDeviceSize memoryOffset);

        /// return true if the Image's data has been modified and should be copied to the buffer.
        bool requiresCopy(uint32_t deviceID) const { return data && data->differentModifiedCount(_vulkanData[deviceID].copiedModifiedCount); }

        /// return true if the Image's data has been modified and should be copied to the buffer, updating the device specific ModifiedCount to the Data's ModifiedCount.
        bool syncModifiedCount(uint32_t deviceID) { return data && data->getModifiedCount(_vulkanData[deviceID].copiedModifiedCount); }

        virtual void compile(Device* device);
        virtual void compile(Context& context);

    protected:
        virtual ~Image();

        struct VulkanData
        {
            VkImage image = VK_NULL_HANDLE;
            ref_ptr<DeviceMemory> deviceMemory;
            VkDeviceSize memoryOffset = 0;
            VkDeviceSize size = 0;
            ref_ptr<Device> device;
            bool requiresDataCopy = false;
            ModifiedCount copiedModifiedCount;

            void release();
        };

        vk_buffer<VulkanData> _vulkanData;
    };
    VSG_type_name(vsg::Image);

} // namespace vsg
