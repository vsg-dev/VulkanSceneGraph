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

ref_ptr<BufferInfo> MemoryBufferPools::reserveBuffer(VkDeviceSize totalSize, VkDeviceSize alignment, VkBufferUsageFlags bufferUsageFlags, VkSharingMode sharingMode, VkMemoryPropertyFlags memoryPropertiesFlags)
{
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

    }

    //debug(name, " : bufferInfo->offset = ", bufferInfo->offset);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*device, bufferInfo->buffer->vk(device->deviceID), &memRequirements);

    auto reservedMemorySlot = reserveMemory(memRequirements, memoryPropertiesFlags);

    if (!reservedMemorySlot.first)
    {
        info(name, " : Completely Failed to space for MemoryBufferPools::reserveBuffer(", totalSize, ", ", alignment, ", ", bufferUsageFlags, ")");
        //throw Exception{"Error: Failed to allocate Buffer from MemoryBufferPool.", VK_ERROR_OUT_OF_DEVICE_MEMORY};
        return {};
    }

    //debug(name, " : Allocated new buffer, MemoryBufferPools::reserveBuffer(", totalSize, ", ", alignment, ", ", bufferUsageFlags, ") ");
    bufferInfo->buffer->bind(reservedMemorySlot.first, reservedMemorySlot.second);

    //if (!bufferInfo->buffer->full())
    {
        std::scoped_lock<std::mutex> lock(_mutex);
        //debug(name, "  inserting new Buffer into Context.bufferPools");
        bufferPools.push_back(bufferInfo->buffer);
    }

    return bufferInfo;
}

MemoryBufferPools::DeviceMemoryOffset MemoryBufferPools::reserveMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags memoryPropertiesFlags, void* pNextAllocInfo)
{
    VkDeviceSize totalSize = memRequirements.size;
    // vsg::info("MemoryBufferPools::reserveMemory() ", totalSize, ", device->availableMemory() = ", device->availableMemory());

    std::scoped_lock<std::mutex> lock(_mutex);

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
        VkDeviceSize availableSpace = std::max(minimumDeviceMemorySize, totalSize);

#if 1
        VkPhysicalDeviceMemoryBudgetPropertiesEXT memoryBudget;
        memoryBudget.sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
        memoryBudget.pNext = nullptr;

        VkPhysicalDeviceMemoryProperties2 dmp;
        dmp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        dmp.pNext = &memoryBudget;

        vkGetPhysicalDeviceMemoryProperties2(*(device->getPhysicalDevice()), &dmp);

        auto& memoryProperties = dmp.memoryProperties;

        uint32_t heapIndex = 0;
        for(uint32_t i=0; i<memoryProperties.memoryTypeCount; ++i)
        {
            if ((memoryProperties.memoryTypes[i].propertyFlags & memoryPropertiesFlags) == memoryPropertiesFlags) // supported
            {
                heapIndex = memoryProperties.memoryTypes[i].heapIndex;
                VkDeviceSize heapAvailable = memoryBudget.heapBudget[heapIndex] - memoryBudget.heapUsage[heapIndex];
                availableSpace = heapAvailable;
                break;
            }
        }

        VkDeviceSize minimumSpare = 0;//16*1024*1024;
        if (availableSpace < minimumSpare) availableSpace = 0;
        else availableSpace -= minimumSpare;
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
                // if (!deviceMemory->full())
                {
                    //debug("  inserting DeviceMemory into memoryPool ", deviceMemory.get());
                    memoryPools.push_back(deviceMemory);
                }
            }
            else
            {
                info("MemoryBufferPools::reserveMemory() A Unsufficent availableSpace = ", availableSpace, ",  for totalSize = ", totalSize);
            }
        }
        else
        {
            info("MemoryBufferPools::reserveMemory()  B Unsufficent availableSpace = ", availableSpace, ",  for totalSize = ", totalSize);
        }
    }

    if (!reservedSlot.first)
    {
        info("MemoryBufferPools::reserveMemory() about to throw exception");
        //throw Exception{"Error: Failed to allocate DeviceMemory from MemoryBufferPool.", VK_ERROR_OUT_OF_DEVICE_MEMORY};
        return {};
    }

    //debug("MemoryBufferPools::reserveMemory() allocated DeviceMemoryOffset(", deviceMemory, ", ", reservedSlot.second, ")");
    return MemoryBufferPools::DeviceMemoryOffset(deviceMemory, reservedSlot.second);
}

VkResult MemoryBufferPools::reserve(ResourceRequirements& requirements)
{
    //vsg::info("MemoryBufferPools::reserve(ResourceRequirements& requirements) { ");

    auto deviceID = device->deviceID;
    auto& limits = device->getPhysicalDevice()->getProperties().limits;

    VkMemoryPropertyFlags memoryPropertiesFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    // allocate bufferInfos
    VkDeviceSize failedBufferMemory = 0;
    for(auto& [properties, bufferInfos] : requirements.bufferInfos)
    {
        for(auto& bufferInfo : bufferInfos)
        {
            if (!bufferInfo->buffer)
            {
                VkDeviceSize alignment = 4;
                if ((properties.usageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) != 0) alignment = limits.minUniformBufferOffsetAlignment;
                else if ((properties.usageFlags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) != 0) alignment = limits.minStorageBufferOffsetAlignment;

                auto newBufferInfo = reserveBuffer(bufferInfo->data->dataSize(), alignment, properties.usageFlags, properties.sharingMode, memoryPropertiesFlags);
                if (newBufferInfo)
                {
                    bufferInfo->buffer = newBufferInfo->buffer;
                    bufferInfo->offset = newBufferInfo->offset;
                    bufferInfo->range = newBufferInfo->range;

                    newBufferInfo->release();

                    //info("    ALLOCATED usage = ", properties.usageFlags, ", alignment = ", alignment, ", bufferInfo->data = ", bufferInfo->data, ", offset = ", bufferInfo->offset, ", range = ", bufferInfo->range, ", buffer = ", bufferInfo->buffer, " ----- size = ", computeSize(*bufferInfo));
                }
                else
                {
                    failedBufferMemory += bufferInfo->data->dataSize();

                    //info("    FAILED to allocate bufferInfo->data = ", bufferInfo->data, ", offset = ", bufferInfo->offset, ", range = ", bufferInfo->range, ", buffer = ", bufferInfo->buffer, " ----- size = ", computeSize(*bufferInfo));
                }
            }
        }
    }

    // allocate images
    VkDeviceSize failedImageMemory = 0;
    for(auto& imageInfo : requirements.imageInfos)
    {
        if (imageInfo->imageView && imageInfo->imageView->image && imageInfo->imageView->image->getDeviceMemory(deviceID) == 0)
        {
            if (imageInfo->imageView->image->compile(*this) == VK_SUCCESS && imageInfo->imageView->image->getDeviceMemory(deviceID) != 0)
            {
                //info("    ALLOCATED imageInfo = ", imageInfo, ", imageView = ", imageInfo->imageView, " ----- size = ", computeSize(*imageInfo), " device memory = ", imageInfo->imageView->image->getDeviceMemory(deviceID), " offset = ", imageInfo->imageView->image->getMemoryOffset(deviceID));
            }
            else
            {
                failedImageMemory += computeSize(*imageInfo);
                //info("    FAILED to allocate imageInfo = ", imageInfo, " ----- size = ", computeSize(*imageInfo));
            }
        }
    }


    VkDeviceSize memoryRequired = failedBufferMemory + failedImageMemory;

    // all required resources allocated
    if (memoryRequired == 0)
    {
        //info("MemoryBufferPools::reserve() memoryRequired = ", memoryRequired);
        return VK_SUCCESS;
    }
    else
    {
        //VkDeviceSize memoryAvailable = device->availableMemory(false);
        //info("MemoryBufferPools::reserve() out of memory: memoryAvailable = ", memoryAvailable, " memoryRequired = ", memoryRequired);
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
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
