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
BufferInfo vsg::copyDataToStagingBuffer(Context& context, const Data* data)
{
    if (!data) return {};

    VkDeviceSize imageTotalSize = data->dataSize();

    VkDeviceSize alignment = std::max(VkDeviceSize(4), VkDeviceSize(data->valueSize()));
    BufferInfo stagingBufferInfo = context.stagingMemoryBufferPools->reserveBuffer(imageTotalSize, alignment, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBufferInfo.data = const_cast<Data*>(data);

    // std::cout<<"stagingBufferInfo.buffer "<<stagingBufferInfo.buffer.get()<<", "<<stagingBufferInfo.offset<<", "<<stagingBufferInfo.range<<")"<<std::endl;

    ref_ptr<Buffer> imageStagingBuffer(stagingBufferInfo.buffer);
    ref_ptr<DeviceMemory> imageStagingMemory(imageStagingBuffer->getDeviceMemory(context.deviceID));

    if (!imageStagingMemory) return {};

    // copy data to staging memory
    imageStagingMemory->copy(imageStagingBuffer->getMemoryOffset(context.deviceID) + stagingBufferInfo.offset, imageTotalSize, data->dataPointer());

    // std::cout << "Creating imageStagingBuffer and memory size = " << imageTotalSize<<std::endl;

    return stagingBufferInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::createBufferAndTransferData
//
BufferInfoList vsg::createBufferAndTransferData(Context& context, const DataList& dataList, VkBufferUsageFlags usage, VkSharingMode sharingMode)
{
    //std::cout<<"\nvsg::createBufferAndTransferData()"<<std::endl;

    //return BufferInfoList();

    //std::cout<<"New vsg::createBufferAndTransferData()"<<std::endl;

    // compute memory requirements

    // create staging buffer
    // create staging memory
    // bind staging buffer
    // map staging memory
    //    copy data to staging memory
    // unmap staging memory
    //
    // create device buffer
    // create device memory
    // submit command to copy from staging buffer to device buffer
    //
    // assign device buffer to BufferInfoList

    Device* device = context.device;

    if (dataList.empty()) return BufferInfoList();

    BufferInfoList bufferInfoList;

    VkDeviceSize alignment = 4;
    if (usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) alignment = device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment;

    VkDeviceSize totalSize = 0;
    VkDeviceSize offset = 0;
    bufferInfoList.reserve(dataList.size());
    for (auto& data : dataList)
    {
        bufferInfoList.push_back(BufferInfo(0, offset, data->dataSize(), data));
        VkDeviceSize endOfEntry = offset + data->dataSize();
        offset = (alignment == 1 || (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
    }

    totalSize = bufferInfoList.back().offset + bufferInfoList.back().range;

    VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;

    BufferInfo deviceBufferInfo = context.deviceMemoryBufferPools->reserveBuffer(totalSize, alignment, bufferUsageFlags, sharingMode, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //std::cout<<"deviceBufferInfo.buffer "<<deviceBufferInfo.buffer.get()<<", "<<deviceBufferInfo.offset<<", "<<deviceBufferInfo.range<<")"<<std::endl;

    // assign the buffer to the bufferData entries
    for (auto& bufferData : bufferInfoList)
    {
        bufferData.buffer = deviceBufferInfo.buffer;
        bufferData.offset += deviceBufferInfo.offset;
    }

    BufferInfo stagingBufferInfo = context.stagingMemoryBufferPools->reserveBuffer(totalSize, alignment, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sharingMode, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    //std::cout<<"stagingBufferInfo.buffer "<<stagingBufferInfo.buffer.get()<<", "<<stagingBufferInfo.offset<<", "<<stagingBufferInfo.range<<")"<<std::endl;

    ref_ptr<Buffer> stagingBuffer(stagingBufferInfo.buffer);
    ref_ptr<DeviceMemory> stagingMemory(stagingBuffer->getDeviceMemory(context.deviceID));

    if (!stagingMemory)
    {
        return BufferInfoList();
    }

    void* buffer_data;
    stagingMemory->map(stagingBuffer->getMemoryOffset(context.deviceID) + stagingBufferInfo.offset, stagingBufferInfo.range, 0, &buffer_data);
    char* ptr = reinterpret_cast<char*>(buffer_data);

    //std::cout<<"    buffer_data " <<buffer_data<<", stagingBufferInfo.offset="<<stagingBufferInfo.offset<<", "<<totalSize<< std::endl;

    for (size_t i = 0; i < dataList.size(); ++i)
    {
        const Data* data = dataList[i];
        std::memcpy(ptr + bufferInfoList[i].offset - deviceBufferInfo.offset, data->dataPointer(), data->dataSize());
    }

    stagingMemory->unmap();

#if 0 // merging of ByfferDataCommands caused problems with later release of individual parts of the buffers copied, so removing this optimization
    CopyAndReleaseBufferInfoCommand* previous = context.copyBufferInfoCommands.empty() ? nullptr : context.copyBufferInfoCommands.back().get();
    if (previous)
    {
        bool sourceMatched = (previous->source._buffer == stagingBufferInfo.buffer) && ((previous->source.offset + previous->source.range) == stagingBufferInfo.offset);
        bool destinationMatched = (previous->destination._buffer == deviceBufferInfo.buffer) && ((previous->destination.offset + previous->destination.range) == deviceBufferInfo.offset);

        if (sourceMatched && destinationMatched)
        {
            //std::cout<<"Source matched = "<<sourceMatched<<" destinationMatched = "<<destinationMatched<<std::endl;
            previous->source.range += stagingBufferInfo.range;
            previous->destination.range += stagingBufferInfo.range;
            return bufferInfoList;
        }
    }
#endif
    context.commands.emplace_back(CopyAndReleaseBuffer::create(stagingBufferInfo, deviceBufferInfo));

    return bufferInfoList;
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
        bufferInfoList.push_back(BufferInfo(0, offset, data->dataSize(), data));
        VkDeviceSize endOfEntry = offset + data->dataSize();
        offset = (alignment == 1 || (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
    }

    totalSize = bufferInfoList.back().offset + bufferInfoList.back().range;

    ref_ptr<Buffer> buffer = vsg::createBufferAndMemory(device, totalSize, usage, sharingMode, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    for (auto& bufferData : bufferInfoList)
    {
        bufferData.buffer = buffer;
    }

    return bufferInfoList;
}

void vsg::copyDataListToBuffers(Device* device, BufferInfoList& bufferInfoList)
{
    for (auto& bufferData : bufferInfoList)
    {
        bufferData.copyDataToBuffer(device->deviceID);
    }
}
