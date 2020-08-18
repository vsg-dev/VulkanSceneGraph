/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/vk/MemoryBufferPools.h>

#include <iostream>

#define REPORT_STATS 0

#if REPORT_STATS
#    include <chrono>
#endif

using namespace vsg;

MemoryBufferPools::MemoryBufferPools(const std::string& in_name, Device* in_device, BufferPreferences preferences) :
    name(in_name),
    device(in_device),
    bufferPreferences(preferences)
{
}

VkDeviceSize MemoryBufferPools::computeMemoryTotalAvailable() const
{
    VkDeviceSize totalAvailableSize = 0;
    for (auto& deviceMemory : memoryPools)
    {
        totalAvailableSize += deviceMemory->memorySlots().totalAvailableSize();
    }
    return totalAvailableSize;
}

VkDeviceSize MemoryBufferPools::computeMemoryTotalReserved() const
{
    VkDeviceSize totalReservedSize = 0;
    for (auto& deviceMemory : memoryPools)
    {
        totalReservedSize += deviceMemory->memorySlots().totalReservedSize();
    }
    return totalReservedSize;
}

VkDeviceSize MemoryBufferPools::computeBufferTotalAvailable() const
{
    VkDeviceSize totalAvailableSize = 0;
    for (auto& buffer : bufferPools)
    {
        totalAvailableSize += buffer->memorySlots().totalAvailableSize();
    }
    return totalAvailableSize;
}

VkDeviceSize MemoryBufferPools::computeBufferTotalReserved() const
{
    VkDeviceSize totalReservedSize = 0;
    for (auto& buffer : bufferPools)
    {
        totalReservedSize += buffer->memorySlots().totalReservedSize();
    }
    return totalReservedSize;
}

BufferData MemoryBufferPools::reserveBufferData(VkDeviceSize totalSize, VkDeviceSize alignment, VkBufferUsageFlags bufferUsageFlags, VkSharingMode sharingMode, VkMemoryPropertyFlags memoryProperties)
{
    BufferData bufferData;
    for (auto& bufferFromPool : bufferPools)
    {
        if (bufferFromPool->usage() == bufferUsageFlags && bufferFromPool->maximumAvailableSpace() >= totalSize)
        {
            MemorySlots::OptionalOffset reservedBufferSlot = bufferFromPool->reserve(totalSize, alignment);
            if (reservedBufferSlot.first)
            {
                bufferData.buffer = bufferFromPool;
                bufferData.offset = reservedBufferSlot.second;
                bufferData.range = totalSize;

#if REPORT_STATS
                std::cout << name << " : MemoryBufferPools::reserveBufferData(" << totalSize << ", " << alignment << ", " << bufferUsageFlags << ") _offset = " << bufferData.offset << std::endl;
#endif
                return bufferData;
            }
        }
    }

#if REPORT_STATS
    std::cout << name << " : Failed to find space in existing buffers with  MemoryBufferPools::reserveBufferData(" << totalSize << ", " << alignment << ", " << bufferUsageFlags << ") bufferPools.size() = " << bufferPools.size() << " looking to allocated new Buffer." << std::endl;
#endif

#if REPORT_STATS
    VkDeviceSize maxAvailableSize = 0;
    VkDeviceSize totalAvailableSize = 0;
    VkDeviceSize totalReservedSize = 0;
    for (auto& buffer : bufferPools)
    {
        if (buffer->maximumAvailableSpace() > maxAvailableSize)
        {
            maxAvailableSize = buffer->maximumAvailableSpace();
        }
        totalAvailableSize += buffer->memorySlots().totalAvailableSize();
        totalReservedSize += buffer->memorySlots().totalReservedSize();
    }
    std::cout << name << " : maxAvailableSize = " << maxAvailableSize << ", totalAvailableSize = " << totalAvailableSize << ", totalReservedSize = " << totalReservedSize << ", totalSize = " << totalSize << ", alignment = " << alignment << std::endl;
#endif

    VkDeviceSize deviceSize = totalSize;

    VkDeviceSize minumumBufferSize = bufferPreferences.minimumBufferSize;
    if (deviceSize < minumumBufferSize)
    {
        deviceSize = minumumBufferSize;
    }

    bufferData.buffer = vsg::Buffer::create(device, deviceSize, bufferUsageFlags, sharingMode);

    MemorySlots::OptionalOffset reservedBufferSlot = bufferData.buffer->reserve(totalSize, alignment);
    bufferData.offset = reservedBufferSlot.second;
    bufferData.range = totalSize;

    // std::cout<<name<<" : Created new Buffer "<<bufferData.buffer.get()<<" totalSize "<<totalSize<<" deviceSize = "<<deviceSize<<std::endl;

    if (!bufferData.buffer->full())
    {
        // std::cout<<name<<"  inserting new Buffer into Context.bufferPools"<<std::endl;
        bufferPools.push_back(bufferData.buffer);
    }

    // std::cout<<name<<" : bufferData.offset = "<<bufferData.offset<<std::endl;

    // TODO need to find proper value for deviceID
    uint32_t deviceID = 0;

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*device, bufferData.buffer->vk(deviceID), &memRequirements);

    ref_ptr<DeviceMemory> deviceMemory;
    MemorySlots::OptionalOffset reservedMemorySlot(false, 0);

    for (auto& memoryFromPool : memoryPools)
    {
        if (memoryFromPool->getMemoryRequirements().memoryTypeBits == memRequirements.memoryTypeBits && memoryFromPool->maximumAvailableSpace() >= deviceSize)
        {
            reservedMemorySlot = memoryFromPool->reserve(deviceSize);
            if (reservedMemorySlot.first)
            {
                deviceMemory = memoryFromPool;
                break;
            }
        }
    }

    if (!deviceMemory)
    {
        VkDeviceSize minumumDeviceMemorySize = bufferPreferences.minimumBufferDeviceMemorySize;

        // clamp to an aligned size
        minumumDeviceMemorySize = ((minumumDeviceMemorySize + memRequirements.alignment - 1) / memRequirements.alignment) * memRequirements.alignment;

        if (memRequirements.size < minumumDeviceMemorySize) memRequirements.size = minumumDeviceMemorySize;

        deviceMemory = vsg::DeviceMemory::create(device, memRequirements, memoryProperties); // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        if (deviceMemory)
        {
            reservedMemorySlot = deviceMemory->reserve(deviceSize);
            if (!deviceMemory->full())
            {
                memoryPools.push_back(deviceMemory);
            }
        }
    }
    else
    {
        if (deviceMemory->full())
        {
            std::cout << name << " : DeviceMemory is full " << deviceMemory.get() << std::endl;
        }
    }

    if (!reservedMemorySlot.first)
    {
        // std::cout<<name<<" : Completely Failed to space for MemoryBufferPools::reserveBufferData("<<totalSize<<", "<<alignment<<", "<<bufferUsageFlags<<") "<<std::endl;
        return BufferData();
    }

    // std::cout<<name<<" : Allocated new buffer, MemoryBufferPools::reserveBufferData("<<totalSize<<", "<<alignment<<", "<<bufferUsageFlags<<") "<<std::endl;
    bufferData.buffer->bind(deviceMemory, reservedMemorySlot.second);

    return bufferData;
}

MemoryBufferPools::DeviceMemoryOffset MemoryBufferPools::reserveMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags memoryProperties, void* pNextAllocInfo)
{
    VkDeviceSize totalSize = memRequirements.size;

    ref_ptr<DeviceMemory> deviceMemory;
    MemorySlots::OptionalOffset reservedSlot(false, 0);

    for (auto& memoryPool : memoryPools)
    {
        if (memoryPool->getMemoryRequirements().memoryTypeBits == memRequirements.memoryTypeBits && memoryPool->maximumAvailableSpace() >= totalSize)
        {
            reservedSlot = memoryPool->reserve(totalSize);
            if (reservedSlot.first)
            {
                deviceMemory = memoryPool;
                break;
            }
        }
    }

    if (!deviceMemory)
    {
        VkDeviceSize minumumDeviceMemorySize = bufferPreferences.minimumImageDeviceMemorySize;

        // clamp to an aligned size
        minumumDeviceMemorySize = ((minumumDeviceMemorySize + memRequirements.alignment - 1) / memRequirements.alignment) * memRequirements.alignment;

        //std::cout<<"Creating new local DeviceMemory"<<std::endl;
        if (memRequirements.size < minumumDeviceMemorySize) memRequirements.size = minumumDeviceMemorySize;

        deviceMemory = vsg::DeviceMemory::create(device, memRequirements, memoryProperties, pNextAllocInfo);
        if (deviceMemory)
        {
            reservedSlot = deviceMemory->reserve(totalSize);
            if (!deviceMemory->full())
            {
                //std::cout<<"  inserting DeviceMemory into memoryPool "<<deviceMemory.get()<<std::endl;
                memoryPools.push_back(deviceMemory);
            }
        }
    }
    else
    {
        if (deviceMemory->full())
        {
            //std::cout<<"DeviceMemory is full "<<deviceMemory.get()<<std::endl;
        }
    }

    if (!reservedSlot.first)
    {
        std::cout << "Failed to reserve slot" << std::endl;
        return DeviceMemoryOffset();
    }

    //std::cout << "MemoryBufferPools::reserveMemory() allocated memory at " << reservedSlot.second << std::endl;
    return MemoryBufferPools::DeviceMemoryOffset(deviceMemory, reservedSlot.second);
}
