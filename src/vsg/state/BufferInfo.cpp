/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/CopyAndReleaseBuffer.h>
#include <vsg/core/compare.h>
#include <vsg/io/Logger.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/vk/Context.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::BufferInfo
//
BufferInfo::BufferInfo()
{
}

BufferInfo::BufferInfo(Data* in_data) :
    data(in_data)
{
}

BufferInfo::BufferInfo(Buffer* in_buffer, VkDeviceSize in_offset, VkDeviceSize in_range, Data* in_data) :
    buffer(in_buffer),
    offset(in_offset),
    range(in_range),
    data(in_data)
{
}

BufferInfo::~BufferInfo()
{
    release();
}

ref_ptr<Object> BufferInfo::clone(const CopyOp& copyop) const
{
    auto new_data = copyop(data);
    if (new_data == data) return ref_ptr<Object>(const_cast<BufferInfo*>(this));

    return BufferInfo::create(new_data);
}

int BufferInfo::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if (data != rhs.data && data && rhs.data)
    {
        if (data->dynamic() || rhs.data->dynamic())
        {
            if (data < rhs.data) return -1;
            return 1; // from checks above it must be that data > rhs.data
        }
    }

    if ((result = compare_pointer(data, rhs.data))) return result;

    /// if one of less buffer is assigned treat as a match as data is the same, and we can reuse any BufferInfo that's been assigned.
    if (!buffer || !rhs.buffer) return 0;

    if ((result = compare_pointer(buffer, rhs.buffer))) return result;
    if ((result = compare_value(offset, rhs.offset))) return result;
    return compare_value(range, rhs.range);
}

void BufferInfo::release()
{
    if (parent)
    {
        parent = {};
    }
    else if (buffer)
    {
        buffer->release(offset, range);
    }

    buffer.reset();
    offset = 0;
    range = 0;
}

void BufferInfo::copyDataToBuffer()
{
    if (!buffer) return;

    for (uint32_t deviceID = 0; deviceID < buffer->sizeVulkanData(); ++deviceID)
    {
        copyDataToBuffer(deviceID);
    }
}

void BufferInfo::copyDataToBuffer(uint32_t deviceID)
{
    if (!buffer) return;

    DeviceMemory* dm = buffer->getDeviceMemory(deviceID);
    if (dm)
    {
        if ((dm->getMemoryPropertyFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
        {
            warn("BufferInfo::copyDataToBuffer() cannot copy data. DeviceMemory does not support direct memory mapping.");

            // you can use dynamic data updates provided by vsg::TransferTask or alternatively, you can implement the following steps:
            // 1. allocate staging buffer
            // 2. copy to staging buffer
            // 3. transfer from staging buffer to device local buffer - use CopyAndReleaseBuffer
            return;
        }

        void* buffer_data;
        VkResult result = dm->map(buffer->getMemoryOffset(deviceID) + offset, range, 0, &buffer_data);
        if (result != 0)
        {
            warn("BufferInfo::copyDataToBuffer() cannot copy data. vkMapMemory(..) failed with result = ", result);
            return;
        }

        char* ptr = reinterpret_cast<char*>(buffer_data);
        std::memcpy(ptr, data->dataPointer(), data->dataSize());

        dm->unmap();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::copyDataToStagingBuffer
//
ref_ptr<BufferInfo> vsg::copyDataToStagingBuffer(Context& context, const Data* data)
{
    if (!data) return {};

    VkDeviceSize imageTotalSize = data->dataSize();

    VkDeviceSize alignment = std::max(VkDeviceSize(4), VkDeviceSize(data->valueSize()));
    auto stagingBufferInfo = context.stagingMemoryBufferPools->reserveBuffer(imageTotalSize, alignment, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBufferInfo->data = const_cast<Data*>(data);

    debug("stagingBufferInfo->buffer ", stagingBufferInfo->buffer.get(), ", ", stagingBufferInfo->offset, ", ", stagingBufferInfo->range, ")");

    ref_ptr<Buffer> imageStagingBuffer(stagingBufferInfo->buffer);
    ref_ptr<DeviceMemory> imageStagingMemory(imageStagingBuffer->getDeviceMemory(context.deviceID));

    if (!imageStagingMemory) return {};

    // copy data to staging memory
    imageStagingMemory->copy(imageStagingBuffer->getMemoryOffset(context.deviceID) + stagingBufferInfo->offset, imageTotalSize, data->dataPointer());

    debug("Creating imageStagingBuffer and memory size = ", imageTotalSize);

    return stagingBufferInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::createBufferAndTransferData
//
bool vsg::createBufferAndTransferData(Context& context, const BufferInfoList& bufferInfoList, VkBufferUsageFlags usage, VkSharingMode sharingMode)
{
    debug("vsg::createBufferAndTransferData(.., )");

    if (bufferInfoList.empty()) return false;

    Device* device = context.device;
    auto deviceID = context.deviceID;
    auto transferTask = context.transferTask.get();
    VkDeviceSize alignment = 4;
    if (usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
        alignment = device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment;
    else if (usage == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
        alignment = device->getPhysicalDevice()->getProperties().limits.minStorageBufferOffsetAlignment;

    //transferTask = nullptr;

    ref_ptr<BufferInfo> deviceBufferInfo;
    size_t numBuffersRequired = 0;
    bool containsMultipleParents = false;
    for (auto& bufferInfo : bufferInfoList)
    {
        if (bufferInfo->data)
        {
            if (bufferInfo->data->getModifiedCount(bufferInfo->copiedModifiedCounts[deviceID]))
            {
                ++numBuffersRequired;
            }
        }

        if (bufferInfo->parent)
        {
            if (deviceBufferInfo && bufferInfo->parent != deviceBufferInfo) containsMultipleParents = true;
            deviceBufferInfo = bufferInfo->parent;
        }
    }

    if (numBuffersRequired == 0)
    {
        debug("\nvsg::createBufferAndTransferData(...) already all compiled. deviceID = ", deviceID);
        return false;
    }

    if (containsMultipleParents)
    {
        warn("vsg::createBufferAndTransferData(...) does not support multiple parent BufferInfo.");
        return false;
    }

    VkDeviceSize totalSize = 0;
    VkDeviceSize offset = 0;
    for (const auto& bufferInfo : bufferInfoList)
    {
        if (bufferInfo->data)
        {
            bufferInfo->offset = offset;
            bufferInfo->range = bufferInfo->data->dataSize();
            VkDeviceSize endOfEntry = offset + bufferInfo->range;
            offset = (alignment == 1 || (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
            //info("  BufferInfo.data = ", bufferInfo->data, ", dynamic = ", bufferInfo->data->getLayout().dynamic);
        }
    }

    totalSize = offset;
    if (totalSize == 0) return false;

    if (deviceBufferInfo && deviceBufferInfo->buffer)
    {
        if (totalSize != deviceBufferInfo->range)
        {
            warn("Existing deviceBufferInfo, ", deviceBufferInfo, ", deviceBufferInfo->range  = ", deviceBufferInfo->range, ", ", totalSize, " NOT compatible");
            return false;
        }
        else
        {
            debug("Existing deviceBufferInfo, ", deviceBufferInfo, ", deviceBufferInfo->range  = ", deviceBufferInfo->range, ", ", totalSize, " with compatible size");

            // make sure the VkBuffer is created
            deviceBufferInfo->buffer->compile(context);

            if (!deviceBufferInfo->buffer->getDeviceMemory(deviceID))
            {
                VkMemoryRequirements memRequirements;
                vkGetBufferMemoryRequirements(*device, deviceBufferInfo->buffer->vk(device->deviceID), &memRequirements);

                auto deviceMemoryOffset = context.deviceMemoryBufferPools->reserveMemory(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                deviceBufferInfo->buffer->bind(deviceMemoryOffset.first, deviceMemoryOffset.second);
            }
        }
    }

    if (!deviceBufferInfo)
    {
        VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;
        deviceBufferInfo = context.deviceMemoryBufferPools->reserveBuffer(totalSize, alignment, bufferUsageFlags, sharingMode, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    debug("deviceBufferInfo->buffer ", deviceBufferInfo->buffer, ", ", deviceBufferInfo->offset, ", ", deviceBufferInfo->range, ")");

    // assign the buffer to the bufferData entries and shift the offsets to offset within the buffer
    for (const auto& bufferInfo : bufferInfoList)
    {
        bufferInfo->buffer = deviceBufferInfo->buffer;
        bufferInfo->offset += deviceBufferInfo->offset;
    }

    if (transferTask)
    {
        vsg::debug("vsg::createBufferAndTransferData(..)");

        for (auto& bufferInfo : bufferInfoList)
        {
            vsg::debug("    ", bufferInfo, ", ", bufferInfo->data, ", ", bufferInfo->buffer, ", ", bufferInfo->offset);
            bufferInfo->data->dirty();
            bufferInfo->parent = deviceBufferInfo;
        }

        transferTask->assign(bufferInfoList);

        return true;
    }

    auto stagingBufferInfo = context.stagingMemoryBufferPools->reserveBuffer(totalSize, alignment, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sharingMode, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    debug("stagingBufferInfo->buffer ", stagingBufferInfo->buffer.get(), ", ", stagingBufferInfo->offset, ", ", stagingBufferInfo->range, ")");

    ref_ptr<Buffer> stagingBuffer(stagingBufferInfo->buffer);
    ref_ptr<DeviceMemory> stagingMemory(stagingBuffer->getDeviceMemory(context.deviceID));

    if (!stagingMemory)
    {
        return false;
    }

    void* buffer_data;
    stagingMemory->map(stagingBuffer->getMemoryOffset(context.deviceID) + stagingBufferInfo->offset, stagingBufferInfo->range, 0, &buffer_data);
    char* ptr = reinterpret_cast<char*>(buffer_data);

    debug("    buffer_data ", buffer_data, ", stagingBufferInfo->offset=", stagingBufferInfo->offset, ", ", totalSize);

    for (const auto& bufferInfo : bufferInfoList)
    {
        const Data* data = bufferInfo->data;
        if (data)
        {
            std::memcpy(ptr + bufferInfo->offset - deviceBufferInfo->offset, data->dataPointer(), data->dataSize());
            if (data->properties.dataVariance == STATIC_DATA_UNREF_AFTER_TRANSFER)
            {
                bufferInfo->data.reset();
            }
        }
        bufferInfo->parent = deviceBufferInfo;
    }

    stagingMemory->unmap();

    context.copy(stagingBufferInfo, deviceBufferInfo);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::createHostVisibleBuffer
//
BufferInfoList vsg::createHostVisibleBuffer(Device* device, const DataList& dataList, VkBufferUsageFlags usage, VkSharingMode sharingMode)
{
    if (dataList.empty()) return BufferInfoList();

    BufferInfoList bufferInfoList;

    VkDeviceSize alignment = 4;
    if (usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
        alignment = device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment;
    else if (usage == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
        alignment = device->getPhysicalDevice()->getProperties().limits.minStorageBufferOffsetAlignment;

    VkDeviceSize totalSize = 0;
    VkDeviceSize offset = 0;
    bufferInfoList.reserve(dataList.size());
    for (auto& data : dataList)
    {
        bufferInfoList.push_back(BufferInfo::create(nullptr, offset, data->dataSize(), data));
        VkDeviceSize endOfEntry = offset + data->dataSize();
        offset = (alignment == 1 || (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
    }

    totalSize = bufferInfoList.back()->offset + bufferInfoList.back()->range;

    ref_ptr<Buffer> buffer = vsg::createBufferAndMemory(device, totalSize, usage, sharingMode, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    for (const auto& bufferData : bufferInfoList)
    {
        bufferData->buffer = buffer;
    }

    return bufferInfoList;
}

void vsg::copyDataListToBuffers(Device* device, BufferInfoList& bufferInfoList)
{
    for (auto& bufferData : bufferInfoList)
    {
        bufferData->copyDataToBuffer(device->deviceID);
    }
}

void vsg::assignVulkanArrayData(uint32_t deviceID, const BufferInfoList& arrays, VulkanArrayData& vkd)
{
    //    info("vsg::assignVulkanArrayData(deviceID = ", deviceID, ", arrays.size() = ", arrays.size(), " vkd.vkBuffers.size() = ", vkd.vkBuffers.size(), ", &vkd ", &vkd);
    vkd.vkBuffers.resize(arrays.size());
    vkd.offsets.resize(arrays.size());

    for (size_t i = 0; i < arrays.size(); ++i)
    {
        const auto& bufferInfo = arrays[i];
        if (bufferInfo->buffer)
        {
            vkd.vkBuffers[i] = bufferInfo->buffer->vk(deviceID);
            vkd.offsets[i] = bufferInfo->offset;
        }
        else
        {
            // error, no buffer to assign
            vkd.vkBuffers[i] = 0;
            vkd.offsets[i] = 0;
        }
    }
}
