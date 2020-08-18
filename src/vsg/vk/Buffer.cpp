/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Options.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/Context.h>

#include <iostream>

#define REPORT_STATS 0

using namespace vsg;

void Buffer::VulkanData::release()
{
    if (buffer)
    {
        vkDestroyBuffer(*device, buffer, device->getAllocationCallbacks());
    }

    if (deviceMemory)
    {
        //deviceMemory->release(memoryOffset, memorySlots.totalMemorySize());
        deviceMemory->release(memoryOffset, size);
    }
}

Buffer::Buffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode) :
    _usage(usage),
    _sharingMode(sharingMode),
    _memorySlots(size)
{
    VulkanData& vd = _vulkanData[device->deviceID];

    vd.device = device;
    vd.size = size;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = sharingMode;

    if (VkResult result = vkCreateBuffer(*device, &bufferInfo, device->getAllocationCallbacks(), &vd.buffer); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create vkBuffer.", result};
    }
}


Buffer::~Buffer()
{
#if REPORT_STATS
    std::cout << "start of Buffer::~Buffer() " << this << std::endl;
#endif

    for(auto& vd : _vulkanData) vd.release();

#if REPORT_STATS
    std::cout << "end of Buffer::~Buffer() " << this << std::endl;
#endif
}

VkMemoryRequirements Buffer::getMemoryRequirements(uint32_t deviceID) const
{
    const VulkanData& vd = _vulkanData[deviceID];

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*vd.device, vd.buffer, &memRequirements);
    return memRequirements;
}

VkResult Buffer::bind(DeviceMemory* deviceMemory, VkDeviceSize memoryOffset)
{
    VulkanData& vd = _vulkanData[deviceMemory->getDevice()->deviceID];

    VkResult result = vkBindBufferMemory(*vd.device, vd.buffer, *deviceMemory, memoryOffset);
    if (result == VK_SUCCESS)
    {
        vd.deviceMemory = deviceMemory;
        vd.memoryOffset = memoryOffset;
        vd.size = maximumAvailableSpace();
    }
    return result;
}
