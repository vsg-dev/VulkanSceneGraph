/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/BufferData.h>
#include <vsg/vk/CommandBuffer.h>

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
    // create device buffer
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
        bufferDataList.push_back(BufferData(0, offset, data->dataSize(), data));
        VkDeviceSize endOfEntry = offset + data->dataSize();
        offset = (alignment == 1 || (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
    }

    totalSize = bufferDataList.back()._offset + bufferDataList.back()._range;

    VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;

    ref_ptr<Buffer> deviceBuffer;
    Buffer::OptionalBufferOffset reservedBufferSlot(false, 0);

    for (auto& bufferFromPool : context.bufferPools)
    {
        if (!bufferFromPool->full() && bufferFromPool->usage() == bufferUsageFlags)
        {
            reservedBufferSlot = bufferFromPool->reserve(totalSize, alignment);
            if (reservedBufferSlot.first)
            {
                deviceBuffer = bufferFromPool;
                break;
            }
        }
    }

    if (!deviceBuffer)
    {
        VkDeviceSize deviceSize = totalSize;

        VkDeviceSize minumumBufferSize = context.bufferPreferences.minimumBufferSize;
        if (deviceSize < minumumBufferSize)
        {
            deviceSize = minumumBufferSize;
        }

        deviceBuffer = vsg::Buffer::create(device, deviceSize, bufferUsageFlags, sharingMode);
        // std::cout<<"Created new Buffer "<<deviceBuffer.get()<<" totalSize "<<totalSize<<" deviceSize = "<<deviceSize<<std::endl;

        reservedBufferSlot = deviceBuffer->reserve(totalSize, alignment);
        if (!deviceBuffer->full())
        {
            // std::cout<<"   inserting new Buffer into Context.bufferPools"<<std::endl;
            context.bufferPools.push_back(deviceBuffer);
        }

        //std::cout<<"   reservedBufferSlot.second = "<<reservedBufferSlot.second<<std::endl;

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(*device, *deviceBuffer, &memRequirements);

        ref_ptr<DeviceMemory> deviceMemory;
        DeviceMemory::OptionalMemoryOffset reservedMemorySlot(false, 0);

        for (auto& memoryFromPool : context.memoryPools)
        {
            if (!memoryFromPool->full() && memoryFromPool->getMemoryRequirements().memoryTypeBits == memRequirements.memoryTypeBits)
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
            VkDeviceSize minumumDeviceMemorySize = context.bufferPreferences.minimumBufferDeviceMemorySize;

            // clamp to an aligned size
            minumumDeviceMemorySize = ((minumumDeviceMemorySize + memRequirements.alignment - 1) / memRequirements.alignment) * memRequirements.alignment;

            if (memRequirements.size < minumumDeviceMemorySize) memRequirements.size = minumumDeviceMemorySize;

            deviceMemory = vsg::DeviceMemory::create(device, memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            if (deviceMemory)
            {
                reservedMemorySlot = deviceMemory->reserve(deviceSize);
                if (!deviceMemory->full())
                {
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

        if (!reservedMemorySlot.first)
        {
            std::cout << "vsg::createBufferAndTransferData(..) Failed to reserve slot" << std::endl;
            return BufferDataList();
        }

        deviceBuffer->bind(deviceMemory, reservedMemorySlot.second);
    }

    // assign the buffer to the bufferData entries
    for (auto& bufferData : bufferDataList)
    {
        bufferData._buffer = deviceBuffer;
    }

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

    // shift the offsets if we are not writing to the start of the deviceBuffer.
    if (reservedBufferSlot.second > 0)
    {
        for (auto& bufferData : bufferDataList)
        {
            bufferData._offset += reservedBufferSlot.second;
        }
    }

    dispatchCommandsToQueue(device, context.commandPool, context.graphicsQueue, [&](VkCommandBuffer transferCommand) {
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = reservedBufferSlot.second;
        copyRegion.size = totalSize;
        vkCmdCopyBuffer(transferCommand, *stagingBuffer, *deviceBuffer, 1, &copyRegion);
    });

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
