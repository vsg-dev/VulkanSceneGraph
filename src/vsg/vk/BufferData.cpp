/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BufferData.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/traversals/CompileTraversal.h>

#include <iostream>


using namespace vsg;


BufferDataList vsg::createBufferAndTransferData(Context& context, const DataList& dataList, VkBufferUsageFlags usage, VkSharingMode sharingMode)
{
    //return BufferDataList();

    //std::cout<<"New vsg::createBufferAndTransferData()"<<std::endl;

    // compute memory requirements

    // create staging buffer
    // create staging memory
    // bind staging buffer
    // map staging memory
    //    copy data to staging memory
    // unmap staging memory
    //
    // crete device buffer
    // create device memory
    // submit command to copy from staging buffer to device buffer
    //
    // assign device buffer to BufferDataList

    Device* device = context.device;

    if (dataList.empty()) return BufferDataList();

    BufferDataList bufferDataList;

    VkDeviceSize alignment = 4;
    if (usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) alignment = device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment;

    VkDeviceSize totalSize = 0;
    VkDeviceSize offset = 0;
    bufferDataList.reserve(dataList.size());
    for (auto& data : dataList)
    {
        bufferDataList.push_back(BufferData(0, offset, data->dataSize()));
        VkDeviceSize endOfEntry = offset + data->dataSize();
        offset = (alignment == 1 || (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
    }

    totalSize = bufferDataList.back()._offset + bufferDataList.back()._range;


    ref_ptr<Buffer> deviceBuffer = vsg::Buffer::create(device, totalSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, sharingMode);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*device, *deviceBuffer, &memRequirements);

    ref_ptr<DeviceMemory> deviceMemory;
    DeviceMemory::OptionalMemoryOffset reservedSlot(false, 0);

    for(auto& memoryPool : context.memoryPools)
    {
        if (!memoryPool->full() && memoryPool->getMemoryRequirements().memoryTypeBits==memRequirements.memoryTypeBits)
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
        VkDeviceSize minumumDeviceMemorySize = context.minimumBufferDeviceMemorySize;

        // clamp to an aligned size
        minumumDeviceMemorySize = ((minumumDeviceMemorySize+memRequirements.alignment-1)/memRequirements.alignment)*memRequirements.alignment;

        //std::cout<<"Creating new local DeviceMemory"<<std::endl;
        if (memRequirements.size<minumumDeviceMemorySize) memRequirements.size = minumumDeviceMemorySize;

        deviceMemory = vsg::DeviceMemory::create(device, memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (deviceMemory)
        {
            reservedSlot = deviceMemory->reserve(totalSize);
            if (!deviceMemory->full())
            {
                //std::cout<<"  inserting DeviceMemory into memoryPool "<<deviceMemory.get()<<std::endl;
                context.memoryPools.push_back(deviceMemory);
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
        std::cout<<"Failed to reserve slot"<<std::endl;
        return BufferDataList();
    }

    //std::cout<<"DeviceMemory "<<deviceMemory.get()<<" slot position = "<<reservedSlot.second<<", size = "<<totalSize<<std::endl;
    deviceBuffer->bind(deviceMemory, reservedSlot.second);

    // assign the buffer to the bufferData entries
    for (auto& bufferData : bufferDataList)
    {
        bufferData._buffer = deviceBuffer;
    }

#if 1

    ref_ptr<Buffer> stagingBuffer = vsg::Buffer::create(device, totalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sharingMode);
    ref_ptr<DeviceMemory> stagingMemory = vsg::DeviceMemory::create(device, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (!stagingMemory)
    {
        return BufferDataList();
    }

    stagingBuffer->bind(stagingMemory, 0);

    void* buffer_data;
    stagingMemory->map(0, totalSize, 0, &buffer_data);
    char* ptr = reinterpret_cast<char*>(buffer_data);

    for (size_t i = 0; i < dataList.size(); ++i)
    {
        const Data* data = dataList[i];
        std::memcpy(ptr + bufferDataList[i]._offset, data->dataPointer(), data->dataSize());
    }

    stagingMemory->unmap();

    dispatchCommandsToQueue(device, context.commandPool, context.graphicsQueue, [&](VkCommandBuffer transferCommand) {
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = totalSize;
        vkCmdCopyBuffer(transferCommand, *stagingBuffer, *deviceBuffer, 1, &copyRegion);
    });
#endif

    return bufferDataList;
}

BufferDataList vsg::createHostVisibleBuffer(Device* device, const DataList& dataList, VkBufferUsageFlags usage, VkSharingMode sharingMode)
{
    if (dataList.empty()) return BufferDataList();

    BufferDataList bufferDataList;

    VkDeviceSize alignment = 4;
    if (usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) alignment = device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment;

    VkDeviceSize totalSize = 0;
    VkDeviceSize offset = 0;
    bufferDataList.reserve(dataList.size());
    for (auto& data : dataList)
    {
        bufferDataList.push_back(BufferData(0, offset, data->dataSize(), data));
        VkDeviceSize endOfEntry = offset + data->dataSize();
        offset = (alignment == 1 || (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
    }

    totalSize = bufferDataList.back()._offset + bufferDataList.back()._range;

    ref_ptr<Buffer> buffer = vsg::Buffer::create(device, totalSize, usage, sharingMode);
    ref_ptr<DeviceMemory> memory = vsg::DeviceMemory::create(device, buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    buffer->bind(memory, 0);

    for (auto& bufferData : bufferDataList)
    {
        bufferData._buffer = buffer;
    }

    return bufferDataList;
}

void vsg::copyDataListToBuffers(BufferDataList& bufferDataList)
{
    for (auto& bufferData : bufferDataList)
    {
        DeviceMemory* dm = bufferData._buffer->getDeviceMemory();

        void* buffer_data;
        dm->map(bufferData._offset, bufferData._range, 0, &buffer_data);

        char* ptr = reinterpret_cast<char*>(buffer_data);
        std::memcpy(ptr, bufferData._data->dataPointer(), bufferData._data->dataSize());

        dm->unmap();
    }
}
