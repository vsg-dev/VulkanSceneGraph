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

        VkDeviceSize availableMemory = device->availableMemory(memoryPropertiesFlags, allocatedMemoryLimit);
        if (totalSize > availableMemory)
        {
            // info("MemoryBufferPools::reserveBuffer(", totalSize, ") insufficient memory ", availableMemory);
            return {};
        }

        VkDeviceSize deviceSize = std::max(totalSize, std::min(availableMemory, minimumBufferSize));

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
        debug(name, " : Failed to space for MemoryBufferPools::reserveBuffer(", totalSize, ", ", alignment, ", ", bufferUsageFlags, ")");
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
        if (((memoryPool->getMemoryRequirements().memoryTypeBits & memRequirements.memoryTypeBits) == memRequirements.memoryTypeBits) &&
            memoryPool->maximumAvailableSpace() >= totalSize)
        {
            reservedSlot = memoryPool->reserve(totalSize, memRequirements.alignment);
            if (reservedSlot.first)
            {
                deviceMemory = memoryPool;
                break;
            }
        }
    }

    if (!deviceMemory)
    {

        VkDeviceSize availableMemory = device->availableMemory(memoryPropertiesFlags, allocatedMemoryLimit);
        if (totalSize > availableMemory)
        {
            debug("MemoryBufferPools::reserveBuffer(", totalSize, ") insufficient memory ", availableMemory);
            return {};
        }

        VkDeviceSize deviceSize = std::max(totalSize, std::min(availableMemory, minimumDeviceMemorySize));

        if (deviceSize <= availableMemory)
        {

            try
            {
                deviceMemory = vsg::DeviceMemory::create(device, memRequirements, memoryPropertiesFlags, pNextAllocInfo);
            }
            catch (...)
            {
                // info("Could not allocate vsg::DeviceMemory(..) memRequirements.size = ", memRequirements.size);
            }

            if (deviceMemory)
            {
                reservedSlot = deviceMemory->reserve(totalSize);
                // if (!deviceMemory->full())
                {
                    //debug("  inserting DeviceMemory into memoryPool ", deviceMemory.get());
                    memoryPools.push_back(deviceMemory);
                }
            }
        }

        // vsg::info(" MemoryBufferPools::reserveMemory(totalSize = ", totalSize, " availableSpace = ", availableSpace, ") deviceMemory = ", deviceMemory);

#if 0
        else if (throwOutOfDeviceMemoryException)
        {
            throw vsg::Exception{"MemoryBufferPools::reserve() out of memory", VK_ERROR_OUT_OF_DEVICE_MEMORY};
        }
#endif
    }

    if (!reservedSlot.first)
    {
        debug("MemoryBufferPools::reserveMemory(", totalSize, ") failed, insufficient memory available.");
        return {};
    }

    //debug("MemoryBufferPools::reserveMemory() allocated DeviceMemoryOffset(", deviceMemory, ", ", reservedSlot.second, ")");
    return MemoryBufferPools::DeviceMemoryOffset(deviceMemory, reservedSlot.second);
}

VkResult MemoryBufferPools::reserve(ResourceRequirements& requirements)
{
    //vsg::info("MemoryBufferPools::reserve(ResourceRequirements& requirements) { ");

    auto deviceID = device->deviceID;
    const auto& limits = device->getPhysicalDevice()->getProperties().limits;

    VkMemoryPropertyFlags memoryPropertiesFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    decltype(requirements.bufferInfos) failed_bufferInfos;
    decltype(requirements.imageInfos) failed_imageInfos;


    // allocate bufferInfos
    bool allocationSuccess = true;
    for (auto& [properties, bufferInfos] : requirements.bufferInfos)
    {
        VkDeviceSize alignment = 4;
        if ((properties.usageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) != 0)
            alignment = limits.minUniformBufferOffsetAlignment;
        else if ((properties.usageFlags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) != 0)
            alignment = limits.minStorageBufferOffsetAlignment;

        for (auto& bufferInfo : bufferInfos)
        {
            if (!bufferInfo->buffer)
            {
                if (allocationSuccess)
                {
                    debug("MemoryBufferPools::reserve(ResourceRequirements& requirements) properties.usageFlags = ", properties.usageFlags, ", alignment = ", alignment);

                    auto newBufferInfo = reserveBuffer(bufferInfo->data->dataSize(), alignment, properties.usageFlags, properties.sharingMode, memoryPropertiesFlags);
                    if (newBufferInfo)
                    {
                        bufferInfo->take(*newBufferInfo);
                    }
                    else
                    {
                        debug("MemoryBufferPools::reserve() failed on ", bufferInfo, ", data = ", bufferInfo->data);
                        allocationSuccess = false;
                    }
                }

                if (!allocationSuccess)
                {
                    failed_bufferInfos[properties].insert(bufferInfo);
                }
            }
        }
    }

    // allocate images
    for (auto& imageInfo : requirements.imageInfos)
    {
        if (imageInfo->imageView && imageInfo->imageView->image && imageInfo->imageView->image->getDeviceMemory(deviceID) == 0)
        {
            if (allocationSuccess)
            {
                auto image_result = imageInfo->imageView->image->compile(*this);
                if (image_result != VK_SUCCESS || imageInfo->imageView->image->getDeviceMemory(deviceID) == 0)
                {
                    debug("MemoryBufferPools::reserve() failed on ", imageInfo, ", data = ", imageInfo->imageView->image->data);
                    allocationSuccess = false;
                }
            }

            if (!allocationSuccess)
            {
                failed_imageInfos.insert(imageInfo);
            }
        }
    }

    // all required resources allocated
    if (allocationSuccess)
    {
        return VK_SUCCESS;
    }
    else
    {
        requirements.bufferInfos.swap(failed_bufferInfos);
        requirements.imageInfos.swap(failed_imageInfos);

        // LogOutput output;
        // report(output);

        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }
}

void MemoryBufferPools::report(LogOutput& out) const
{
    out.enter("MemoryBufferPools::report(..)");
    for (const auto& memoryPool : memoryPools)
    {
        memoryPool->report(out);
    }
    out.leave();
}
