/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/vk/Buffer.h>
#include <vsg/io/Options.h>

#include <iostream>

#define REPORT_STATS 0

using namespace vsg;

Buffer::Buffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, AllocationCallbacks* allocator) :
    _usage(usage),
    _sharingMode(sharingMode),
    _device(device),
    _allocator(allocator),
    _memorySlots(size)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = sharingMode;

    if (VkResult result = vkCreateBuffer(*device, &bufferInfo, allocator, &_buffer); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create vkBuffer.", result};
    }
}

Buffer::~Buffer()
{
#if REPORT_STATS
    std::cout << "start of Buffer::~Buffer() " << this << std::endl;
#endif

    if (_buffer)
    {
        vkDestroyBuffer(*_device, _buffer, _allocator);
    }

    if (_deviceMemory)
    {
        _deviceMemory->release(_memoryOffset, _memorySlots.totalMemorySize());
    }
#if REPORT_STATS
    std::cout << "end of Buffer::~Buffer() " << this << std::endl;
#endif
}

VkMemoryRequirements Buffer::getMemoryRequirements() const
{
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*_device, _buffer, &memRequirements);
    return memRequirements;
}
