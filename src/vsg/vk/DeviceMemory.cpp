/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Buffer.h>
#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/Image.h>

#include <cstring>

#include <iostream>

using namespace vsg;

///////////////////////////////////////////////////////////////////////////////
//
// MemorySlots
//
MemorySlots::MemorySlots(VkDeviceSize availableMemorySize)
{
    _availableMemory.insert(SizeOffset(availableMemorySize, 0));
    _offsetSizes.insert(OffsetSize(0, availableMemorySize));
}

MemorySlots::OptionalOffset MemorySlots::reserve(VkDeviceSize size, VkDeviceSize alignment)
{
    if (full()) return OptionalOffset(false, 0);

    auto itr = _availableMemory.lower_bound(size);
    while (itr != _availableMemory.end())
    {
        SizeOffset slot(*itr);
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
                _availableMemory.insert(SizeOffset(preAlignedStartSize, slotStart));
                _offsetSizes.insert(OffsetSize(slotStart, preAlignedStartSize));
            }

            // check if there is space at the end slot that isn't used completely, if so generate an available space for it.
            if (alignedSize < slotSize)
            {
                // insert new slot with new end and new size
                VkDeviceSize postAlignedEndSize = slotSize-alignedSize;
                _availableMemory.insert(SizeOffset(postAlignedEndSize, alignedEnd));
                _offsetSizes.insert(OffsetSize(alignedEnd, postAlignedEndSize));
            }

            return OptionalOffset(true, alignedStart);
        }
        else
        {
//            std::cout << "    Slot slotStart = " << slotStart << ", slotSize = " << slotSize << " not big enough once for request size = " << size << std::endl;
            ++itr;
        }
    }

    return OptionalOffset(false, 0);
}

void MemorySlots::release(VkDeviceSize offset, VkDeviceSize size)
{
    if (_offsetSizes.empty())
    {
        // first empty space
        _availableMemory.insert(SizeOffset(size, offset));
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

    _availableMemory.insert(SizeOffset(size, offset));
    _offsetSizes.insert(OffsetSize(offset, size));
}

///////////////////////////////////////////////////////////////////////////////
//
// DeviceMemory
//
DeviceMemory::DeviceMemory(VkDeviceMemory deviceMemory, const VkMemoryRequirements& memRequirements, Device* device, AllocationCallbacks* allocator) :
    _deviceMemory(deviceMemory),
    _memoryRequirements(memRequirements),
    _device(device),
    _allocator(allocator),
    _memorySlots(memRequirements.size)
{
}

DeviceMemory::~DeviceMemory()
{
    if (_deviceMemory)
    {
        vkFreeMemory(*_device, _deviceMemory, _allocator);
    }
}

DeviceMemory::Result DeviceMemory::create(Device* device, const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return DeviceMemory::Result("Error: vsg::DeviceMemory::create(...) failed to create DeviceMemory, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    uint32_t typeFilter = memRequirements.memoryTypeBits;

    // find the memory type to use
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(*(device->getPhysicalDevice()), &memProperties);
    uint32_t i;
    for (i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) break;
    }
    if (i >= memProperties.memoryTypeCount)
    {
        return DeviceMemory::Result("Error: vsg::DeviceMemory::create(...) failed to create DeviceMemory, not usage memory type found.", VK_ERROR_FORMAT_NOT_SUPPORTED);
    }
    uint32_t memoryTypeIndex = i;

#if 0
    if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    {
        static VkDeviceSize s_TotalDeviceMemoryAllocated = 0;
        s_TotalDeviceMemoryAllocated += memRequirements.size;
        std::cout<<"Device Local DeviceMemory::DeviceMemory() "<<memRequirements.size<<", "<<memRequirements.alignment<<", "<<memRequirements.memoryTypeBits<<",  s_TotalMemoryAllocated = "<<s_TotalDeviceMemoryAllocated<<std::endl;
    }
    else
    {
        static VkDeviceSize s_TotalHostMemoryAllocated = 0;
        s_TotalHostMemoryAllocated += memRequirements.size;
        std::cout<<"Staging DeviceMemory::DeviceMemory() "<<memRequirements.size<<", "<<memRequirements.alignment<<", "<<memRequirements.memoryTypeBits<<",  s_TotalMemoryAllocated = "<<s_TotalHostMemoryAllocated<<std::endl;
    }
#endif

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memRequirements.size;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;

    VkDeviceMemory deviceMemory;
    VkResult result = vkAllocateMemory(*device, &allocateInfo, allocator, &deviceMemory);
    if (result == VK_SUCCESS)
    {
        return Result(new DeviceMemory(deviceMemory, memRequirements, device, allocator));
    }
    else
    {
        return Result("Error: Failed to create DeviceMemory.", result);
    }
}

DeviceMemory::Result DeviceMemory::create(Device* device, Buffer* buffer, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator)
{
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*device, *buffer, &memRequirements);

    return vsg::DeviceMemory::create(device, memRequirements, properties, allocator);
}

DeviceMemory::Result DeviceMemory::create(Device* device, Image* image, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator)
{
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(*device, *image, &memRequirements);

    return vsg::DeviceMemory::create(device, memRequirements, properties, allocator);
}

VkResult DeviceMemory::map(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
{
    return vkMapMemory(*_device, _deviceMemory, offset, size, flags, ppData);
}

void DeviceMemory::unmap()
{
    vkUnmapMemory(*_device, _deviceMemory);
}

void DeviceMemory::copy(VkDeviceSize offset, VkDeviceSize size, const void* src_data)
{
    // should we have checks against buffer having enough memory for copied data?

    void* buffer_data;
    map(offset, size, 0, &buffer_data);

    std::memcpy(buffer_data, src_data, (size_t)size);

    unmap();
}
