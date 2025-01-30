/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/CopyAndReleaseBuffer.h>
#include <vsg/io/Logger.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

CopyAndReleaseBuffer::CopyAndReleaseBuffer(ref_ptr<MemoryBufferPools> optional_stagingMemoryBufferPools) :
    stagingMemoryBufferPools(optional_stagingMemoryBufferPools)
{
}

CopyAndReleaseBuffer::~CopyAndReleaseBuffer()
{
}

void CopyAndReleaseBuffer::copy(ref_ptr<Data> data, ref_ptr<BufferInfo> dest)
{
    VkDeviceSize dataSize = data->dataSize();
    VkDeviceSize alignment = std::max(VkDeviceSize(4), VkDeviceSize(data->valueSize()));

    //debug("CopyAndReleaseImage::copyDirectly() dataSize = ", dataSize);

    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    ref_ptr<BufferInfo> stagingBufferInfo = stagingMemoryBufferPools->reserveBuffer(dataSize, alignment, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, memoryPropertyFlags);
    stagingBufferInfo->data = data;

    //debug("stagingBufferInfo->buffer ", stagingBufferInfo->buffer.get(), ", ", stagingBufferInfo->offset, ", ", stagingBufferInfo->range, ")");

    auto deviceID = stagingMemoryBufferPools->device->deviceID;
    ref_ptr<Buffer> imageStagingBuffer(stagingBufferInfo->buffer);
    ref_ptr<DeviceMemory> stagingMemory(imageStagingBuffer->getDeviceMemory(deviceID));

    if (!stagingMemory) return;

    // copy data to staging memory
    stagingMemory->copy(imageStagingBuffer->getMemoryOffset(deviceID) + stagingBufferInfo->offset, dataSize, data->dataPointer());

    add(stagingBufferInfo, dest);
}

void CopyAndReleaseBuffer::add(ref_ptr<BufferInfo> src, ref_ptr<BufferInfo> dest)
{
    std::scoped_lock lock(_mutex);
    _pending.push_back(CopyData{src, dest});
}

void CopyAndReleaseBuffer::CopyData::record(CommandBuffer& commandBuffer) const
{
    //debug("CopyAndReleaseBuffer::CopyData::record(CommandBuffer& commandBuffer) source.offset = ", source->offset, ", ", destination->offset);
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = source->offset;
    copyRegion.dstOffset = destination->offset;
    copyRegion.size = source->range;
    vkCmdCopyBuffer(commandBuffer, source->buffer->vk(commandBuffer.deviceID), destination->buffer->vk(commandBuffer.deviceID), 1, &copyRegion);
}

void CopyAndReleaseBuffer::record(CommandBuffer& commandBuffer) const
{
    std::scoped_lock lock(_mutex);

    _readyToClear.clear();

    _readyToClear.swap(_completed);

    for (const auto& copyData : _pending)
    {
        copyData.record(commandBuffer);
    }

    _pending.swap(_completed);
}
