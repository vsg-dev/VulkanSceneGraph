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
    _offsetSizes.insert(OffsetSize(0, size));
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
        VkDeviceSize alignedEnd = alignedStart + size;
        VkDeviceSize alignedSize = alignedEnd - slotStart;

        if (alignedSize <= slotSize)
        {
            // remove slot
            _availableMemory.erase(itr);
            if (auto offsetSize_itr = _offsetSizes.find(slotStart); offsetSize_itr !=  _offsetSizes.end()) _offsetSizes.erase(offsetSize_itr);

            // check if there the front of the slot isn't used completely, if so generate an available space for it.
            if (alignedStart > slotStart)
            {
                // insert new slot with previous slots start and new end.
                VkDeviceSize preAlignedStartSize = alignedStart-slotStart;
                _availableMemory.insert(MemorySlot(preAlignedStartSize, slotStart));
                _offsetSizes.insert(OffsetSize(slotStart, preAlignedStartSize));
            }

            // check if there is space at the end slot that isn't used completely, if so generate an available space for it.
            if (alignedSize < slotSize)
            {
                // insert new slot with new end and new size
                VkDeviceSize postAlignedEndSize = slotSize-alignedSize;
                _availableMemory.insert(MemorySlot(postAlignedEndSize, alignedEnd));
                _offsetSizes.insert(OffsetSize(alignedEnd, postAlignedEndSize));
            }

            return OptionalBufferOffset(true, alignedStart);
        }
        else
        {
//            std::cout << "    Slot slotStart = " << slotStart << ", slotSize = " << slotSize << " not big enough once for request size = " << size << std::endl;
            ++itr;
        }
    }

    return OptionalBufferOffset(false, 0);
}

void Buffer::release(VkDeviceSize offset, VkDeviceSize size)
{
    if (_offsetSizes.empty())
    {
        // first empty space
        _availableMemory.insert(MemorySlot(size, offset));
        _offsetSizes.insert(OffsetSize(offset, size));
        return;
    }

    // need to find adjacent blocks before and after to see if we abut so we can join them togeher options are:
    //    abutes to neither before or after
    //    abutes to before, so replace before with new combined legnth
    //    abutes to after, so remove after entry and insert new enty with combined length
    //    abutes to both before and after, so replace before with newly combined length of all three, remove after entry

    auto slotAfter = _offsetSizes.upper_bound(offset);
    auto slotBefore = slotAfter;
    if (slotBefore != _offsetSizes.end()) --slotBefore;
    else slotBefore = _offsetSizes.rbegin().base();

    auto eraseSlot = [&] (OffsetSizes::iterator offsetSizeItr)
    {
        auto range = _availableMemory.equal_range(offsetSizeItr->second);
        for(auto itr = range.first; itr != range.second; ++itr)
        {
            if (itr->second == offsetSizeItr->first)
            {
                _availableMemory.erase(itr);
                _offsetSizes.erase(offsetSizeItr);
                break;
            }
        }
    };

    if (slotBefore != _offsetSizes.end())
    {
        VkDeviceSize endOfBeforeSlot = slotBefore->first + slotBefore->second;

        if (endOfBeforeSlot == offset)
        {
            VkDeviceSize endOfReleasedSlot = offset + size;
            VkDeviceSize totalSizeOfMergedSlots = endOfReleasedSlot - slotBefore->first;

            offset = slotBefore->first;
            size = totalSizeOfMergedSlots;

            eraseSlot(slotBefore);
        }
    }
    if (slotAfter != _offsetSizes.end())
    {
        VkDeviceSize endOfReleasedSlot = offset + size;

        if (endOfReleasedSlot == slotAfter->first)
        {
            VkDeviceSize endOfSlotAfter = slotAfter->first + slotAfter->second;
            VkDeviceSize totalSizeOfMergedSlots = endOfSlotAfter - offset;
            size = totalSizeOfMergedSlots;

            eraseSlot(slotAfter);
        }
    }

    _availableMemory.insert(MemorySlot(size, offset));
    _offsetSizes.insert(OffsetSize(offset, size));
}
