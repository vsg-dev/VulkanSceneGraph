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
    class VSG_DECLSPEC Image : public Inherit<Object, Image>
    {
    public:

        Image(VkImage image, Device* device);
        Image(Device* device, const VkImageCreateInfo& createImageInfo);

        VkImage vk(uint32_t deviceID) const { return _vulkanData[deviceID].image; }

        DeviceMemory* getDeviceMemory(uint32_t deviceID) { return _vulkanData[deviceID].deviceMemory; }
        const DeviceMemory* getDeviceMemory(uint32_t deviceID) const { return _vulkanData[deviceID].deviceMemory; }

        VkDeviceSize getMemoryOffset(uint32_t deviceID) const { return _vulkanData[deviceID].memoryOffset; }

        VkMemoryRequirements getMemoryRequirements(uint32_t deviceID) const;

        VkResult bind(DeviceMemory* deviceMemory, VkDeviceSize memoryOffset);

    protected:
        virtual ~Image();

        struct VulkanData
        {
            VkImage image = VK_NULL_HANDLE;
            ref_ptr<DeviceMemory> deviceMemory;
            VkDeviceSize memoryOffset = 0;
            ref_ptr<Device> device;

            void release();
        };

        vk_buffer<VulkanData> _vulkanData;
    };
    VSG_type_name(vsg::Image);

} // namespace vsg
