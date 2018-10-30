/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BufferData.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

BufferDataList vsg::createBufferAndTransferData(Device* device, CommandPool* commandPool, VkQueue graphicsQueue, const DataList& dataList, VkBufferUsageFlags usage, VkSharingMode sharingMode)
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
        bufferDataList.push_back(BufferData(0, offset, data->dataSize()));
        VkDeviceSize endOfEntry = offset + data->dataSize();
        offset = (alignment == 1 || (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
    }

    totalSize = bufferDataList.back()._offset + bufferDataList.back()._range;

    ref_ptr<Buffer> stagingBuffer = vsg::Buffer::create(device, totalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sharingMode);
    ref_ptr<DeviceMemory> stagingMemory = vsg::DeviceMemory::create(device, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
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

    ref_ptr<Buffer> deviceBuffer = vsg::Buffer::create(device, totalSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, sharingMode);
    ref_ptr<DeviceMemory> deviceMemory = vsg::DeviceMemory::create(device, deviceBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    deviceBuffer->bind(deviceMemory, 0);

    dispatchCommandsToQueue(device, commandPool, graphicsQueue, [&](VkCommandBuffer transferCommand) {
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = totalSize;
        vkCmdCopyBuffer(transferCommand, *stagingBuffer, *deviceBuffer, 1, &copyRegion);
    });

    // assign the buffer to the bufferData entries
    for (auto& bufferData : bufferDataList)
    {
        bufferData._buffer = deviceBuffer;
    }

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
