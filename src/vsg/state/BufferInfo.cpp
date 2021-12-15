/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/CopyAndReleaseBuffer.h>
#include <vsg/io/Options.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>

#include <iostream>

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

    buffer = 0;
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
            std::cout << "Warning: BufferInfo::copyDataToBuffer() cannot copy data. DeviceMemory does not support direct memory mapping." << std::endl;

            // 1. allocate staging buffer
            // 2. copy to staging buffer
            // 3. transfer from staging buffer to device local buffer - use CopyAndReleaseBuffer

            return;
        }

        void* buffer_data;
        VkResult result = dm->map(offset, range, 0, &buffer_data);
        if (result != 0)
        {
            std::cout << "Warning: BufferInfo::copyDataToBuffer() cannot copy data. VkMapMemory(..) failed with result = " << result << std::endl;
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

    // std::cout<<"stagingBufferInfo->buffer "<<stagingBufferInfo->buffer.get()<<", "<<stagingBufferInfo->offset<<", "<<stagingBufferInfo->range<<")"<<std::endl;

    ref_ptr<Buffer> imageStagingBuffer(stagingBufferInfo->buffer);
    ref_ptr<DeviceMemory> imageStagingMemory(imageStagingBuffer->getDeviceMemory(context.deviceID));

    if (!imageStagingMemory) return {};

    // copy data to staging memory
    imageStagingMemory->copy(imageStagingBuffer->getMemoryOffset(context.deviceID) + stagingBufferInfo->offset, imageTotalSize, data->dataPointer());

    // std::cout << "Creating imageStagingBuffer and memory size = " << imageTotalSize<<std::endl;

    return stagingBufferInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::createBufferAndTransferData
//
bool vsg::createBufferAndTransferData(Context& context, const BufferInfoList& bufferInfoList, VkBufferUsageFlags usage, VkSharingMode sharingMode)
{
    //std::cout<<"vsg::createBufferAndTransferData(.., )"<<std::endl;

    if (bufferInfoList.empty()) return false;

    auto deviceID = context.deviceID;

    ref_ptr<BufferInfo> deviceBufferInfo;
    size_t numBuffersAssigned = 0;
    size_t numBuffersRequired = 0;
    size_t numNoData = 0;
    bool containsMultipleParents = false;
    for (auto& bufferInfo : bufferInfoList)
    {
        if (bufferInfo->data)
        {
            if (bufferInfo->data->getModifiedCount(bufferInfo->copiedModifiedCounts[deviceID]))
            {
                ++numBuffersRequired;
            }
            else
            {
                ++numBuffersAssigned;
            }
        }
        else
        {
            ++numNoData;
        }

        if (bufferInfo->parent)
        {
            if (deviceBufferInfo && bufferInfo->parent != deviceBufferInfo) containsMultipleParents = true;
            deviceBufferInfo = bufferInfo->parent;
        }
    }

    if (numBuffersRequired == 0)
    {
        std::cout << "\nvsg::createBufferAndTransferData(...) already all compiled. deviceID = " << deviceID << std::endl;
        return false;
    }

    if (containsMultipleParents)
    {
        std::cout << "Warning : vsg::createBufferAndTransferData(...) does not support multiple parent BufferInfo." << std::endl;
        return false;
    }

    Device* device = context.device;

    VkDeviceSize alignment = 4;
    if (usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) alignment = device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment;

    VkDeviceSize totalSize = 0;
    VkDeviceSize offset = 0;
    for (auto& bufferInfo : bufferInfoList)
    {
        if (bufferInfo->data)
        {
            bufferInfo->offset = offset;
            bufferInfo->range = bufferInfo->data->dataSize();
            VkDeviceSize endOfEntry = offset + bufferInfo->range;
            offset = (alignment == 1 || (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
        }
    }

    totalSize = offset;
    if (totalSize == 0) return false;

    if (deviceBufferInfo && deviceBufferInfo->buffer)
    {
        if (totalSize != deviceBufferInfo->range)
        {
            std::cout << "Exisitng deviceBufferInfo, " << deviceBufferInfo << ", deviceBufferInfo->range  = " << deviceBufferInfo->range << ", " << totalSize << " NOT compatible" << std::endl;
            return false;
        }
        else
        {
            //std::cout<<"Exisitng deviceBufferInfo, "<<deviceBufferInfo<<", deviceBufferInfo->range  = "<<deviceBufferInfo->range <<", "<<totalSize<<" with compatible size"<<std::endl;

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

    //std::cout<<"deviceBufferInfo->buffer "<<deviceBufferInfo->buffer.get()<<", "<<deviceBufferInfo->offset<<", "<<deviceBufferInfo->range<<")"<<std::endl;

    // assign the buffer to the bufferData entries and shift the offsets to offset within the buffer
    for (auto& bufferInfo : bufferInfoList)
    {
        bufferInfo->buffer = deviceBufferInfo->buffer;
        bufferInfo->offset += deviceBufferInfo->offset;
    }

    auto stagingBufferInfo = context.stagingMemoryBufferPools->reserveBuffer(totalSize, alignment, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sharingMode, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    //std::cout<<"stagingBufferInfo->buffer "<<stagingBufferInfo->buffer.get()<<", "<<stagingBufferInfo->offset<<", "<<stagingBufferInfo->range<<")"<<std::endl;

    ref_ptr<Buffer> stagingBuffer(stagingBufferInfo->buffer);
    ref_ptr<DeviceMemory> stagingMemory(stagingBuffer->getDeviceMemory(context.deviceID));

    if (!stagingMemory)
    {
        return false;
    }

    void* buffer_data;
    stagingMemory->map(stagingBuffer->getMemoryOffset(context.deviceID) + stagingBufferInfo->offset, stagingBufferInfo->range, 0, &buffer_data);
    char* ptr = reinterpret_cast<char*>(buffer_data);

    //std::cout<<"    buffer_data " <<buffer_data<<", stagingBufferInfo->offset="<<stagingBufferInfo->offset<<", "<<totalSize<< std::endl;

    for (auto& bufferInfo : bufferInfoList)
    {
        const Data* data = bufferInfo->data;
        if (data)
        {
            std::memcpy(ptr + bufferInfo->offset - deviceBufferInfo->offset, data->dataPointer(), data->dataSize());
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
    if (usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) alignment = device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment;

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

    for (auto& bufferData : bufferInfoList)
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
