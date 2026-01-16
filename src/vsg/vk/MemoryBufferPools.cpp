/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/vk/MemoryBufferPools.h>

#include <algorithm>
#include <chrono>

using namespace vsg;

MemoryBufferPools::MemoryBufferPools(const std::string& in_name, ref_ptr<Device> in_device, const ResourceRequirements& in_resourceRequirements) :
    name(in_name),
    device(in_device),
    minimumBufferSize(in_resourceRequirements.minimumBufferSize),
    minimumDeviceMemorySize(in_resourceRequirements.minimumDeviceMemorySize)
{
}

VkDeviceSize MemoryBufferPools::computeMemoryTotalAvailable() const
{
    std::scoped_lock<std::mutex> lock(_mutex);

    VkDeviceSize totalAvailableSize = 0;
    for (auto& deviceMemory : memoryPools)
    {
        totalAvailableSize += deviceMemory->totalAvailableSize();
    }
    return totalAvailableSize;
}

VkDeviceSize MemoryBufferPools::computeMemoryTotalReserved() const
{
    std::scoped_lock<std::mutex> lock(_mutex);

    VkDeviceSize totalReservedSize = 0;
    for (auto& deviceMemory : memoryPools)
    {
        totalReservedSize += deviceMemory->totalReservedSize();
    }
    return totalReservedSize;
}

VkDeviceSize MemoryBufferPools::computeBufferTotalAvailable() const
{
    std::scoped_lock<std::mutex> lock(_mutex);

    VkDeviceSize totalAvailableSize = 0;
    for (auto& buffer : bufferPools)
    {
        totalAvailableSize += buffer->totalAvailableSize();
    }
    return totalAvailableSize;
}

VkDeviceSize MemoryBufferPools::computeBufferTotalReserved() const
{
    std::scoped_lock<std::mutex> lock(_mutex);

    VkDeviceSize totalReservedSize = 0;
    for (auto& buffer : bufferPools)
    {
        totalReservedSize += buffer->totalReservedSize();
    }
    return totalReservedSize;
}

ref_ptr<BufferInfo> MemoryBufferPools::reserveBuffer(VkDeviceSize totalSize, VkDeviceSize alignment, VkBufferUsageFlags bufferUsageFlags, VkSharingMode sharingMode, VkMemoryPropertyFlags memoryProperties)
{
    vsg::info("MemoryBufferPools::reserveBuffer(totalSize = ", totalSize, ", alignment = ", alignment,")");

    ref_ptr<BufferInfo> bufferInfo = BufferInfo::create();

    {
        std::scoped_lock<std::mutex> lock(_mutex);
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
                    return bufferInfo;
                }
            }
        }

        VkDeviceSize deviceSize = std::max(totalSize, minimumBufferSize);

        bufferInfo->buffer = Buffer::create(deviceSize, bufferUsageFlags, sharingMode);
        bufferInfo->buffer->compile(device);

        MemorySlots::OptionalOffset reservedBufferSlot = bufferInfo->buffer->reserve(totalSize, alignment);
        bufferInfo->offset = reservedBufferSlot.second;
        bufferInfo->range = totalSize;

        //debug(name, " : Created new Buffer ", bufferInfo->buffer.get(), " totalSize ", totalSize, " deviceSize = ", deviceSize);

        if (!bufferInfo->buffer->full())
        {
            //debug(name, "  inserting new Buffer into Context.bufferPools");
            bufferPools.push_back(bufferInfo->buffer);
        }
    }

    //debug(name, " : bufferInfo->offset = ", bufferInfo->offset);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*device, bufferInfo->buffer->vk(device->deviceID), &memRequirements);

    auto reservedMemorySlot = reserveMemory(memRequirements, memoryProperties);

    if (!reservedMemorySlot.first)
    {
        //debug(name, " : Completely Failed to space for MemoryBufferPools::reserveBuffer(", totalSize, ", ", alignment, ", ", bufferUsageFlags, ") ");
        return {};
    }

    //debug(name, " : Allocated new buffer, MemoryBufferPools::reserveBuffer(", totalSize, ", ", alignment, ", ", bufferUsageFlags, ") ");
    bufferInfo->buffer->bind(reservedMemorySlot.first, reservedMemorySlot.second);

    return bufferInfo;
}

MemoryBufferPools::DeviceMemoryOffset MemoryBufferPools::reserveMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags memoryPropertiesFlags, void* pNextAllocInfo)
{
    std::scoped_lock<std::mutex> lock(_mutex);

    ref_ptr<DeviceMemory> deviceMemory;
    VkDeviceSize totalSize = memRequirements.size;
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
        // vsg::info("device->availableMemory() = ", device->availableMemory());

        VkPhysicalDeviceMemoryBudgetPropertiesEXT memoryBudget;
        memoryBudget.sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
        memoryBudget.pNext = nullptr;

        VkPhysicalDeviceMemoryProperties2 dmp;
        dmp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        dmp.pNext = &memoryBudget;

        vkGetPhysicalDeviceMemoryProperties2(*(device->getPhysicalDevice()), &dmp);

        auto& memoryProperties = dmp.memoryProperties;

        VkDeviceSize availableSpace = 0;
        for(uint32_t i=0; i<memoryProperties.memoryTypeCount; ++i)
        {
            if ((memoryProperties.memoryTypes[i].propertyFlags & memoryPropertiesFlags) == memoryPropertiesFlags) // supported
            {
                uint32_t heapIndex = memoryProperties.memoryTypes[i].heapIndex;
                VkDeviceSize heapAvailable = memoryBudget.heapBudget[heapIndex] - memoryBudget.heapUsage[heapIndex];
                availableSpace = heapAvailable;
            }
        }
#if 1
        vsg::info("MemoryBufferPools::reserveMemory() memoryTypeCount = ", memoryProperties.memoryTypeCount , ", memoryHeapCount = ", memoryProperties.memoryHeapCount);
        for(uint32_t i=0; i<memoryProperties.memoryTypeCount; ++i)
        {
            vsg::info("   propertyFlags = ", memoryProperties.memoryTypes[i].propertyFlags , ", heapIndex = ", memoryProperties.memoryTypes[i].heapIndex);
        }


        for(uint32_t i=0; i<memoryProperties.memoryHeapCount; ++i)
        {
            double percentage = 100.0*(static_cast<double>(memoryBudget.heapUsage[i])/static_cast<double>(memoryBudget.heapBudget[i]));
            vsg::info("    heapUsage = ", memoryBudget.heapUsage[i], "  heapBudget = ", memoryBudget.heapBudget[i], ", ", percentage, "%" );
        }
#endif

        if (totalSize <= availableSpace)
        {
            if (availableSpace < minimumDeviceMemorySize)
            {
                info("Reducing minimumDeviceMemorySize = ", minimumDeviceMemorySize, " to ", availableSpace);
                minimumDeviceMemorySize = availableSpace;
            }

            VkDeviceSize deviceMemorySize = std::max(totalSize, minimumDeviceMemorySize);

            // clamp to an aligned size
            deviceMemorySize = ((deviceMemorySize + memRequirements.alignment - 1) / memRequirements.alignment) * memRequirements.alignment;

            //debug("Creating new local DeviceMemory");
            if (memRequirements.size < deviceMemorySize) memRequirements.size = deviceMemorySize;

            deviceMemory = vsg::DeviceMemory::create(device, memRequirements, memoryPropertiesFlags, pNextAllocInfo);
            if (deviceMemory)
            {
                reservedSlot = deviceMemory->reserve(totalSize);
                if (!deviceMemory->full())
                {
                    //debug("  inserting DeviceMemory into memoryPool ", deviceMemory.get());
                    memoryPools.push_back(deviceMemory);
                }
            }
        }
        else
        {
            info("Unsufficent availableSpace = ", availableSpace, ",  for totalSize = ", totalSize);
        }
    }
    else
    {
        if (deviceMemory->full())
        {
            //debug("DeviceMemory is full ", deviceMemory.get());
        }
    }

    if (!reservedSlot.first)
    {
        //debug("MemoryBufferPools::reserveMemory() Failed to reserve slot");
        return {};
    }

    //debug("MemoryBufferPools::reserveMemory() allocated DeviceMemoryOffset(", deviceMemory, ", ", reservedSlot.second, ")");
    return MemoryBufferPools::DeviceMemoryOffset(deviceMemory, reservedSlot.second);
}

VkResult MemoryBufferPools::reserve(ResourceRequirements& requirements)
{
    VkDeviceSize memoryAvailable = device->availableMemory();
    VkDeviceSize memoryMargin = 1024 * 1024 * 1024;
    //VkDeviceSize memoryMargin = 512 * 1024 * 1024;
    VkDeviceSize memoryRequired = requirements.bufferMemoryRequirements + requirements.imageMemoryRequirements;

    // reserving buffers require
    // reserveBuffer(VkDeviceSize totalSize, VkDeviceSize alignment, VkBufferUsageFlags bufferUsageFlags, VkSharingMode sharingMode, VkMemoryPropertyFlags memoryProperties)

    auto deviceID = device->deviceID;
    VkDeviceSize bufferMemory = 0;
    VkDeviceSize imageMemory = 0;
    info("Context::reserve() memory requirements OK : memoryAvailable = ", memoryAvailable, " memoryRequired = ", memoryRequired, " with memoryMargin = ", memoryRequired + memoryMargin);
    info("  requirements.bufferInfos.size() = ", requirements.bufferInfos.size());
    for(auto& bufferInfo : requirements.bufferInfos)
    {
        if (!bufferInfo->buffer)
        {
            bufferMemory += computeSize(*bufferInfo);
            info("    bufferInfo->data = ", bufferInfo->data, ", offset = ", bufferInfo->offset, ", range = ", bufferInfo->range, ", buffer = ", bufferInfo->buffer, " ----- size = ", computeSize(*bufferInfo));
        }
        else
        {
            info("    compiled bufferInfo->data = ", bufferInfo->data, ", offset = ", bufferInfo->offset, ", range = ", bufferInfo->range, ", buffer = ", bufferInfo->buffer, " ----- size = ", computeSize(*bufferInfo));
        }
    }
    info("  computed bufferMemory = ", bufferMemory, " vs requirements ", requirements.bufferMemoryRequirements);
    info("  requirements.imageInfos.size() = ", requirements.imageInfos.size());
    for(auto& imageInfo : requirements.imageInfos)
    {
        if (imageInfo->imageView->vk(deviceID) != 0)
        {
            imageMemory += computeSize(*imageInfo);
            info("    imageInfo = ", imageInfo, " +++++ size = ", computeSize(*imageInfo));
        }
        else
        {
            info("    compiled imageInfo = ", imageInfo, " +++++ size = ", computeSize(*imageInfo));
        }
    }
    info("  computed imageMemory = ", imageMemory, " vs requirements ", requirements.imageMemoryRequirements);

    memoryRequired = bufferMemory + imageMemory;

    if ((memoryRequired + memoryMargin) > memoryAvailable)
    {
        info("Context::reserve() out of memory: memoryAvailable = ", memoryAvailable, " memoryRequired = ", memoryRequired, " with memoryMargin = ", memoryRequired + memoryMargin);
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }
    else
    {

        return VK_SUCCESS;
    }
}

VkDeviceSize MemoryBufferPools::computeSize(BufferInfo& bufferInfo) const
{
    return (bufferInfo.data) ? bufferInfo.data->dataSize(): 0;
}

VkDeviceSize MemoryBufferPools::computeSize(ImageInfo& imageInfo) const
{
    if (imageInfo.imageView && imageInfo.imageView->image)
    {
        auto& image = imageInfo.imageView->image;

        // VkExtent3D extent = {0, 0, 0};
        // uint32_t mipLevels = 0;
        // uint32_t arrayLayers = 0;

        if (image->data) return image->data->computeValueCountIncludingMipmaps();
    }
    return 0;
}
