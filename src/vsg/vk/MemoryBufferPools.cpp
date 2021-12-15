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

MemoryBufferPools::MemoryBufferPools(const std::string& in_name, ref_ptr<Device> in_device, const ResourceRequirements& in_resourceRequirements) :
    name(in_name),
    device(in_device),
    resourceRequirements(in_resourceRequirements)
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

ref_ptr<BufferInfo> MemoryBufferPools::reserveBuffer(VkDeviceSize totalSize, VkDeviceSize alignment, VkBufferUsageFlags bufferUsageFlags, VkSharingMode sharingMode, VkMemoryPropertyFlags memoryProperties)
{
    ref_ptr<BufferInfo> bufferInfo = BufferInfo::create();
    for (auto& bufferFromPool : bufferPools)
    {
        if (bufferFromPool->usage == bufferUsageFlags && bufferFromPool->size >= totalSize)
        {
            MemorySlots::OptionalOffset reservedBufferSlot = bufferFromPool->reserve(totalSize, alignment);
            if (reservedBufferSlot.first)
            {
                bufferInfo->buffer = bufferFromPool;
                bufferInfo->offset = reservedBufferSlot.second;
                bufferInfo->range = totalSize;

#if REPORT_STATS
                std::cout << name << " : MemoryBufferPools::reserveBuffer(" << totalSize << ", " << alignment << ", " << bufferUsageFlags << ") bufferInfo.buffer = " << bufferInfo->buffer << ", offset = " << bufferInfo->offset << std::endl;
#endif
                return bufferInfo;
            }
        }
    }

#if REPORT_STATS
    std::cout << name << " : Failed to find space in existing buffers with  MemoryBufferPools::reserveBuffer(" << totalSize << ", " << alignment << ", " << bufferUsageFlags << ") bufferPools.size() = " << bufferPools.size() << " looking to allocated new Buffer." << std::endl;
#endif

    VkDeviceSize deviceSize = totalSize;

    VkDeviceSize minumumBufferSize = resourceRequirements.minimumBufferSize;
    if (deviceSize < minumumBufferSize)
    {
        deviceSize = minumumBufferSize;
    }

    bufferInfo->buffer = Buffer::create(device, deviceSize, bufferUsageFlags, sharingMode);

    MemorySlots::OptionalOffset reservedBufferSlot = bufferInfo->buffer->reserve(totalSize, alignment);
    bufferInfo->offset = reservedBufferSlot.second;
    bufferInfo->range = totalSize;

    // std::cout<<name<<" : Created new Buffer "<<bufferInfo.buffer.get()<<" totalSize "<<totalSize<<" deviceSize = "<<deviceSize<<std::endl;

    if (!bufferInfo->buffer->full())
    {
        // std::cout<<name<<"  inserting new Buffer into Context.bufferPools"<<std::endl;
        bufferPools.push_back(bufferInfo->buffer);
    }

    // std::cout<<name<<" : bufferInfo->offset = "<<bufferInfo->offset<<std::endl;

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*device, bufferInfo->buffer->vk(device->deviceID), &memRequirements);

    auto reservedMemorySlot = reserveMemory(memRequirements, memoryProperties);

    if (!reservedMemorySlot.first)
    {
        // std::cout<<name<<" : Completely Failed to space for MemoryBufferPools::reserveBuffer("<<totalSize<<", "<<alignment<<", "<<bufferUsageFlags<<") "<<std::endl;
        return {};
    }

    // std::cout<<name<<" : Allocated new buffer, MemoryBufferPools::reserveBuffer("<<totalSize<<", "<<alignment<<", "<<bufferUsageFlags<<") "<<std::endl;
    bufferInfo->buffer->bind(reservedMemorySlot.first, reservedMemorySlot.second);

    return bufferInfo;
}

MemoryBufferPools::DeviceMemoryOffset MemoryBufferPools::reserveMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags memoryProperties, void* pNextAllocInfo)
{
    VkDeviceSize totalSize = memRequirements.size;

    ref_ptr<DeviceMemory> deviceMemory;
    MemorySlots::OptionalOffset reservedSlot(false, 0);

    for (auto& memoryPool : memoryPools)
    {
        if (memoryPool->getMemoryRequirements().memoryTypeBits == memRequirements.memoryTypeBits &&
            memoryPool->getMemoryRequirements().alignment == memRequirements.alignment &&
            memoryPool->maximumAvailableSpace() >= totalSize)
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
        VkDeviceSize minumumDeviceMemorySize = resourceRequirements.minimumImageDeviceMemorySize;

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
        return {};
    }

    //std::cout << "MemoryBufferPools::reserveMemory() allocated DeviceMemoryOffset(" << deviceMemory<<", "<<reservedSlot.second << ")"<<std::endl;
    return MemoryBufferPools::DeviceMemoryOffset(deviceMemory, reservedSlot.second);
}
