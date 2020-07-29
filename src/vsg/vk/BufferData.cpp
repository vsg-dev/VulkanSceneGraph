/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/CopyAndReleaseBufferDataCommand.h>
#include <vsg/io/Options.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/BufferData.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::createBufferAndTransferData
//
BufferDataList vsg::createBufferAndTransferData(Context& context, const DataList& dataList, VkBufferUsageFlags usage, VkSharingMode sharingMode)
{
    //std::cout<<"\nvsg::createBufferAndTransferData()"<<std::endl;

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

    totalSize = bufferDataList.back().offset + bufferDataList.back().range;

    VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;

    BufferData deviceBufferData = context.deviceMemoryBufferPools->reserveBufferData(totalSize, alignment, bufferUsageFlags, sharingMode, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //std::cout<<"deviceBufferData.buffer "<<deviceBufferData.buffer.get()<<", "<<deviceBufferData.offset<<", "<<deviceBufferData.range<<")"<<std::endl;

    // assign the buffer to the bufferData entries
    for (auto& bufferData : bufferDataList)
    {
        bufferData.buffer = deviceBufferData.buffer;
        bufferData.offset += deviceBufferData.offset;
    }

    BufferData stagingBufferData = context.stagingMemoryBufferPools->reserveBufferData(totalSize, alignment, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sharingMode, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    //std::cout<<"stagingBufferData.buffer "<<stagingBufferData.buffer.get()<<", "<<stagingBufferData.offset<<", "<<stagingBufferData.range<<")"<<std::endl;

    ref_ptr<Buffer> stagingBuffer(stagingBufferData.buffer);
    ref_ptr<DeviceMemory> stagingMemory(stagingBuffer->getDeviceMemory());

    if (!stagingMemory)
    {
        return BufferDataList();
    }

    void* buffer_data;
    stagingMemory->map(stagingBuffer->getMemoryOffset() + stagingBufferData.offset, stagingBufferData.range, 0, &buffer_data);
    char* ptr = reinterpret_cast<char*>(buffer_data);

    //std::cout<<"    buffer_data " <<buffer_data<<", stagingBufferData.offset="<<stagingBufferData.offset<<", "<<totalSize<< std::endl;

    for (size_t i = 0; i < dataList.size(); ++i)
    {
        const Data* data = dataList[i];
        std::memcpy(ptr + bufferDataList[i].offset - deviceBufferData.offset, data->dataPointer(), data->dataSize());
    }

    stagingMemory->unmap();

#if 0 // merging of ByfferDataCommands caused problems with later release of individual parts of the buffers copied, so removing this optimization
    CopyAndReleaseBufferDataCommand* previous = context.copyBufferDataCommands.empty() ? nullptr : context.copyBufferDataCommands.back().get();
    if (previous)
    {
        bool sourceMatched = (previous->source._buffer == stagingBufferData.buffer) && ((previous->source.offset + previous->source.range) == stagingBufferData.offset);
        bool destinationMatched = (previous->destination._buffer == deviceBufferData.buffer) && ((previous->destination.offset + previous->destination.range) == deviceBufferData.offset);

        if (sourceMatched && destinationMatched)
        {
            //std::cout<<"Source matched = "<<sourceMatched<<" destinationMatched = "<<destinationMatched<<std::endl;
            previous->source.range += stagingBufferData.range;
            previous->destination.range += stagingBufferData.range;
            return bufferDataList;
        }
    }
#endif
    context.commands.emplace_back(new CopyAndReleaseBufferDataCommand(stagingBufferData, deviceBufferData));

    return bufferDataList;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::createHostVisibleBuffer
//
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

    totalSize = bufferDataList.back().offset + bufferDataList.back().range;

    ref_ptr<Buffer> buffer = vsg::Buffer::create(device, totalSize, usage, sharingMode);
    ref_ptr<DeviceMemory> memory = vsg::DeviceMemory::create(device, buffer->getMemoryRequirements(), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    buffer->bind(memory, 0);

    for (auto& bufferData : bufferDataList)
    {
        bufferData.buffer = buffer;
    }

    return bufferDataList;
}

void vsg::copyDataListToBuffers(BufferDataList& bufferDataList)
{
    for (auto& bufferData : bufferDataList)
    {
        DeviceMemory* dm = bufferData.buffer->getDeviceMemory();

        void* buffer_data;
        dm->map(bufferData.offset, bufferData.range, 0, &buffer_data);

        char* ptr = reinterpret_cast<char*>(buffer_data);
        std::memcpy(ptr, bufferData.data->dataPointer(), bufferData.data->dataSize());

        dm->unmap();
    }
}
