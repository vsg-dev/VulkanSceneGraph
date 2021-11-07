/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Options.h>
#include <vsg/vk/DeviceMemory.h>

#include <atomic>
#include <cstring>
#include <iostream>

using namespace vsg;

#define DO_CHECK 0

///////////////////////////////////////////////////////////////////////////////
//
// MemorySlots
//
MemorySlots::MemorySlots(VkDeviceSize availableMemorySize)
{
    _availableMemory.insert(SizeOffset(availableMemorySize, 0));
    _offsetSizes.insert(OffsetSize(0, availableMemorySize));

    _totalMemorySize = availableMemorySize;
}

VkDeviceSize MemorySlots::totalAvailableSize() const
{
    VkDeviceSize totalSize = 0;
    for (const auto& sizeOffset : _availableMemory)
    {
        totalSize += sizeOffset.first;
    }
    return totalSize;
}

VkDeviceSize MemorySlots::totalReservedSize() const
{
    VkDeviceSize totalSize = 0;
    for (const auto& sizeOffset : _reservedOffsetSizes)
    {
        totalSize += sizeOffset.second;
    }
    return totalSize;
}

bool MemorySlots::check() const
{
    if (_availableMemory.size() != _offsetSizes.size())
    {
        std::cout << "Warning: MemorySlots::check() _availableMemory.size() " << _availableMemory.size() << " != _offsetSizes.size() " << _offsetSizes.size() << std::endl;
    }

    VkDeviceSize availableSize = 0;
    for (auto& offsetSize : _offsetSizes)
    {
        availableSize += offsetSize.second;
    }

    VkDeviceSize reservedSize = 0;
    for (auto& offsetSize : _reservedOffsetSizes)
    {
        reservedSize += offsetSize.second;
    }

    VkDeviceSize computedSize = availableSize + reservedSize;
    if (computedSize != _totalMemorySize)
    {
        std::cout << "Warning : MemorySlots::check() " << this << " failed, computeedSize (" << computedSize << ") != _totalMemorySize (" << _totalMemorySize << ")" << std::endl;

        report();

        throw "MemorySlots check failed";

        return false;
    }

    return true;
}

void MemorySlots::report() const
{
    std::cout << "MemorySlots::report()" << std::endl;
    for (auto& [offset, size] : _offsetSizes)
    {
        std::cout << "    available " << offset << ", " << size << std::endl;
    }

    for (auto& [offset, size] : _reservedOffsetSizes)
    {
        std::cout << "    reserved " << std::dec << offset << ", " << size << std::endl;
    }
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
        VkDeviceSize slotEnd = slotStart + slotSize;

        VkDeviceSize alignedStart = ((slotStart + alignment - 1) / alignment) * alignment;
        VkDeviceSize alignedEnd = alignedStart + size;
        VkDeviceSize alignedSize = alignedEnd - slotStart;

        if (alignedSize <= slotSize)
        {
            // remove slot
            _availableMemory.erase(itr);
            if (auto offsetSize_itr = _offsetSizes.find(slotStart); offsetSize_itr != _offsetSizes.end()) _offsetSizes.erase(offsetSize_itr);

            // check if there the front of the slot isn't used completely, if so generate an available space for it.
            if (alignedStart > slotStart)
            {
                // insert new slot with previous slots start and new end.
                VkDeviceSize preAlignedStartSize = alignedStart - slotStart;
                _availableMemory.insert(SizeOffset(preAlignedStartSize, slotStart));
                _offsetSizes.insert(OffsetSize(slotStart, preAlignedStartSize));
            }

            // check if there is space at the end slot that isn't used completely, if so generate an available space for it.
            if (alignedEnd < slotEnd)
            {
                // insert new slot with new end and new size
                VkDeviceSize postAlignedEndSize = slotEnd - alignedEnd;
                _availableMemory.insert(SizeOffset(postAlignedEndSize, alignedEnd));
                _offsetSizes.insert(OffsetSize(alignedEnd, postAlignedEndSize));
            }

            _reservedOffsetSizes[alignedStart] = (alignedEnd - alignedStart);

#if DO_CHECK
            check();
#endif

            return OptionalOffset(true, alignedStart);
        }
        else
        {
            // std::cout << "    Slot slotStart = " << slotStart << ", slotSize = " << slotSize << " not big enough once for request size = " << size << std::endl;
            ++itr;
        }
    }

    //std::cout<<"MemorySlots::reserve("<<std::dec<<size<<") with alingment "<<alignment<<" No slots available for this size, biggest available slot is : "<<_availableMemory.rbegin()->first<<std::endl;
    //report();

    return OptionalOffset(false, 0);
}

void MemorySlots::release(VkDeviceSize offset, VkDeviceSize size)
{
    auto reserved_itr = _reservedOffsetSizes.find(offset);
    if (reserved_itr == _reservedOffsetSizes.end())
    {
#if DO_CHECK
        std::cout << "   MemorySlots::release() slot not found" << std::endl;
#endif
        return;
    }
    else
    {
#if DO_CHECK
        if (reserved_itr->second != size)
        {
            std::cout << "    MemorySlots::release() slot found but sizes are inconsistent reserved_itr->second = " << std::dec << reserved_itr->second << ", size=" << size << std::endl;
            if (size != 0) throw "MemorySlots::release() slot found but sizes are inconsistent reserved_itr->second";
        }
        else
        {
            std::cout << "    MemorySlots::release() slot found " << std::endl;
        }
#endif

        size = reserved_itr->second;

        _reservedOffsetSizes.erase(reserved_itr);
    }

    if (_offsetSizes.empty())
    {
        // first empty space
        _availableMemory.insert(SizeOffset(size, offset));
        _offsetSizes.insert(OffsetSize(offset, size));

#if DO_CHECK
        check();
#endif
        return;
    }

    // need to find adjacent blocks before and after to see if we can join them together options are:
    //    abutes to neither before or after
    //    abutes to before, so replace before with new combined length
    //    abutes to after, so remove after entry and insert new entry with combined length
    //    abutes to both before and after, so replace before with newly combined length of all three, remove after entry

    auto slotAfter = _offsetSizes.upper_bound(offset);

    auto slotBefore = slotAfter;
    if (slotBefore != _offsetSizes.end())
    {
        if (slotBefore == _offsetSizes.begin())
        {
            slotBefore = _offsetSizes.end();
        }
        else
        {
            --slotBefore;
        }
    }
    else
    {
        slotBefore = _offsetSizes.rbegin().base();
    }

    auto eraseSlot = [&](OffsetSizes::iterator offsetSizeItr) {
        auto range = _availableMemory.equal_range(offsetSizeItr->second);
        for (auto itr = range.first; itr != range.second; ++itr)
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

    //report();

#if DO_CHECK
    check();
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
// DeviceMemory
//
DeviceMemory::DeviceMemory(Device* device, const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties, void* pNextAllocInfo) :
    _memoryRequirements(memRequirements),
    _properties(properties),
    _device(device),
    _memorySlots(memRequirements.size)
{
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
        throw Exception{"Error: vsg::DeviceMemory::create(...) failed to create DeviceMemory, not usage memory type found.", VK_ERROR_FORMAT_NOT_SUPPORTED};
    }
    uint32_t memoryTypeIndex = i;

#if DO_CHECK
    if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    {
        static VkDeviceSize s_TotalDeviceMemoryAllocated = 0;
        s_TotalDeviceMemoryAllocated += memRequirements.size;
        std::cout << "Device Local DeviceMemory::DeviceMemory() " << std::dec << memRequirements.size << ", " << memRequirements.alignment << ", " << memRequirements.memoryTypeBits << ",  s_TotalMemoryAllocated = " << s_TotalDeviceMemoryAllocated << std::endl;
    }
    else
    {
        static VkDeviceSize s_TotalHostMemoryAllocated = 0;
        s_TotalHostMemoryAllocated += memRequirements.size;
        std::cout << "Staging DeviceMemory::DeviceMemory()  " << std::dec << memRequirements.size << ", " << memRequirements.alignment << ", " << memRequirements.memoryTypeBits << ",  s_TotalMemoryAllocated = " << s_TotalHostMemoryAllocated << std::endl;
    }
#endif

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memRequirements.size;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;
    allocateInfo.pNext = pNextAllocInfo;

    if (VkResult result = vkAllocateMemory(*device, &allocateInfo, _device->getAllocationCallbacks(), &_deviceMemory); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to allocate DeviceMemory.", result};
    }
}

DeviceMemory::~DeviceMemory()
{
    if (_deviceMemory)
    {
#if DO_CHECK
        std::cout << "DeviceMemory::~DeviceMemory() vkFreeMemory(*_device, " << _deviceMemory << ", _allocator);" << std::endl;
#endif

        vkFreeMemory(*_device, _deviceMemory, _device->getAllocationCallbacks());
    }
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
