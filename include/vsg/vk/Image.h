#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/DeviceMemory.h>

namespace vsg
{

    class VSG_DECLSPEC Image : public Inherit<Object, Image>
    {
    public:
        Image(VkImage image, Device* device);
        Image(Device* device, const VkImageCreateInfo& createImageInfo);

        VkImage image() const { return _image; }

        operator VkImage() const { return _image; }

        struct Settings
        {
            VkImageCreateFlags       flags;
            VkImageType              imageType;
            VkFormat                 format;
            VkExtent3D               extent;
            uint32_t                 mipLevels;
            uint32_t                 arrayLayers;
            VkSampleCountFlagBits    samples;
            VkImageTiling            tiling;
            VkImageUsageFlags        usage;
            VkSharingMode            sharingMode;
            std::vector<uint32_t>    queueFamilyIndices;
            VkImageLayout            initialLayout;
        };

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

        DeviceMemory* getDeviceMemory() { return _deviceMemory; }
        const DeviceMemory* getDeviceMemory() const { return _deviceMemory; }

        VkDeviceSize getMemoryOffset() const { return _memoryOffset; }

        VkMemoryRequirements getMemoryRequirements() const;

        VkResult bind(DeviceMemory* deviceMemory, VkDeviceSize memoryOffset)
        {
            VkResult result = vkBindImageMemory(*_device, _image, *deviceMemory, memoryOffset);
            if (result == VK_SUCCESS)
            {
                _deviceMemory = deviceMemory;
                _memoryOffset = memoryOffset;
            }
            return result;
        }

        VkImage vk(uint32_t /*deviceID*/) const { return _image; }


    protected:
        virtual ~Image();

        VkImage _image;
        ref_ptr<Device> _device;

        ref_ptr<DeviceMemory> _deviceMemory;
        VkDeviceSize _memoryOffset;
    };
    VSG_type_name(vsg::Image);

} // namespace vsg
