/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Buffer.h>

#include <iostream>

using namespace vsg;

Buffer::Buffer(VkBuffer buffer, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, Device* device, AllocationCallbacks* allocator) :
    _buffer(buffer),
    _usage(usage),
    _sharingMode(sharingMode),
    _device(device),
    _allocator(allocator)
{
    _availableMemory.insert(MemorySlot(size, 0));
}

Buffer::~Buffer()
{
    if (_buffer)
    {
        vkDestroyBuffer(*_device, _buffer, _allocator);
    }
}

Buffer::Result Buffer::create(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Buffer::Result("Error: vsg::Buffer::create(...) failed to create vkBuffer, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = sharingMode;

    VkBuffer buffer;
    VkResult result = vkCreateBuffer(*device, &bufferInfo, allocator, &buffer);
    if (result == VK_SUCCESS)
    {
        return Result(new Buffer(buffer, size, usage, sharingMode, device, allocator));
    }
    else
    {
        return Result("Error: Failed to create vkBuffer.", result);
    }
}

Buffer::OptionalBufferOffset Buffer::reserve(VkDeviceSize size, VkDeviceSize alignment)
{
    if (full()) return OptionalBufferOffset(false, 0);

    auto itr = _availableMemory.lower_bound(size);
    while (itr != _availableMemory.end())
    {
        MemorySlot slot(*itr);
        VkDeviceSize slotStart = slot.second;
        VkDeviceSize slotSize = slot.first;

        VkDeviceSize alignedStart = ((slotStart + alignment - 1) / alignment) * alignment;
        if (((alignedStart-slotStart)+size) <= slotSize)
        {
            VkDeviceSize alignedEnd = ((alignedStart + size + alignment - 1) / alignment) * alignment;
            VkDeviceSize alignedSize = alignedEnd - slotStart;

            _availableMemory.erase(itr);

            //std::cout<<"size = "<<size<<", alignedEnd = "<<alignedEnd<<std::endl;

            if (alignedEnd < slot.first)
            {
                MemorySlot slotUnused(slotSize - alignedSize, alignedEnd);
                _availableMemory.insert(slotUnused);
                //std::cout<<"   slot unused position = " <<slotUnused.second<<" size = "<<slotUnused.first<<", "<<std::endl;
            }
            else
            {
                //std::cout<<"   slot completely used "<<_availableMemory.size()<<std::endl;
            }
            return OptionalBufferOffset(true, slot.second);
        }
        else
        {
            std::cout<<"Slot slotStart = "<<slotStart<<", slotSize = "<<slotSize<<" not big enough once for request size = "<<size<<std::endl;
            ++itr;
        }
    }

    return OptionalBufferOffset(false, 0);
}
