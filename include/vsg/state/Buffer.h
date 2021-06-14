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

    class VSG_DECLSPEC Buffer : public Inherit<Object, Buffer>
    {
    public:
        Buffer(VkDeviceSize in_size, VkBufferUsageFlags in_usage, VkSharingMode in_sharingMode);
        Buffer(Device* device, VkDeviceSize in_size, VkBufferUsageFlags in_usage, VkSharingMode in_sharingMode);

        // VkBufferCreateInfo settings
        VkBufferCreateFlags flags = 0;
        VkDeviceSize size;
        VkBufferUsageFlags usage;
        VkSharingMode sharingMode;

        /// Vulkan VkImage handle
        VkBuffer vk(uint32_t deviceID) const { return _vulkanData[deviceID].buffer; }

        /// return the number of VulkanData entries.
        uint32_t sizeVulkanData() const { return _vulkanData.size(); }

        VkResult bind(DeviceMemory* deviceMemory, VkDeviceSize memoryOffset);

        MemorySlots::OptionalOffset reserve(VkDeviceSize in_size, VkDeviceSize alignment) { return _memorySlots.reserve(in_size, alignment); }
        void release(VkDeviceSize offset, VkDeviceSize in_size) { _memorySlots.release(offset, in_size); }
        bool full() const { return _memorySlots.full(); }
        const MemorySlots& memorySlots() const { return _memorySlots; }

        VkMemoryRequirements getMemoryRequirements(uint32_t deviceID) const;

        DeviceMemory* getDeviceMemory(uint32_t deviceID) { return _vulkanData[deviceID].deviceMemory; }
        const DeviceMemory* getDeviceMemory(uint32_t deviceID) const { return _vulkanData[deviceID].deviceMemory; }

        VkDeviceSize getMemoryOffset(uint32_t deviceID) const { return _vulkanData[deviceID].memoryOffset; }

        virtual bool compile(Device* device);
        virtual bool compile(Context& context);

    protected:
        virtual ~Buffer();

        struct VulkanData
        {
            VkBuffer buffer = VK_NULL_HANDLE;
            ref_ptr<DeviceMemory> deviceMemory;
            VkDeviceSize memoryOffset = 0;
            VkDeviceSize size = 0;
            ref_ptr<Device> device;

            void release();
        };

        vk_buffer<VulkanData> _vulkanData;

        MemorySlots _memorySlots;
    };
    VSG_type_name(vsg::Buffer);

    extern VSG_DECLSPEC ref_ptr<Buffer> createBufferAndMemory(Device* device, VkDeviceSize in_size, VkBufferUsageFlags in_usage, VkSharingMode in_sharingMode, VkMemoryPropertyFlags memoryProperties);

} // namespace vsg
