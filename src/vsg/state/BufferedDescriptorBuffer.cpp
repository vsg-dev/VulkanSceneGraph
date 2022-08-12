/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/BufferedDescriptorBuffer.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/vk/Context.h>

using namespace vsg;

size_t padBufferSize(size_t originalSize, VkDeviceSize alignment)
{
    size_t alignedSize = originalSize;
    if (alignment > 0)
        alignedSize = (alignedSize + alignment - 1) & ~(alignment - 1);
    return alignedSize;
}
size_t bufferSize(vsg::Data &data, VkDeviceSize alignment, size_t numBuffers)
{
    return numBuffers * padBufferSize(data.dataSize(), alignment);
}

BufferedDescriptorBuffer::~BufferedDescriptorBuffer()
{
}

void BufferedDescriptorBuffer::advanceFrame()
{
    _frameIndex = (_frameIndex+1)%numBuffers;
    for (auto& bi : bufferInfoList)
    {
        if (!bi->buffer)
            continue;
        bi->offset = bi->parent->offset + bi->range * static_cast<uint32_t>(_frameIndex);
    }
}
void BufferedDescriptorBuffer::copyDataListToBuffers()
{
    advanceFrame();
    Inherit::copyDataListToBuffers();
}

void BufferedDescriptorBuffer::compile(Context &context)
{
    // assign buffer of appripriate size prior to letting superclass compile.
    bool requiresAssignmentOfBuffers = false;
    for (auto& bufferInfo : bufferInfoList)
    {
        if (bufferInfo->buffer == nullptr) requiresAssignmentOfBuffers = true;
    }

    if (!requiresAssignmentOfBuffers)
    {
        Inherit::compile(context);
        return;
    }

    VkDeviceSize alignment = 4;
    VkBufferUsageFlags bufferUsageFlags = 0;
    if (descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
    {
        bufferUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        alignment = context.device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment;
    }
    else
    {
        bufferUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        alignment = context.device->getPhysicalDevice()->getProperties().limits.minStorageBufferOffsetAlignment;
    }

    VkDeviceSize totalSize = 0;

    // compute the total size of BufferInfo that needs to be allocated.
    {
        VkDeviceSize offset = 0;
        for (auto& bufferInfo : bufferInfoList)
        {
            if (bufferInfo->data && !bufferInfo->buffer)
            {
                totalSize = offset + bufferSize(*bufferInfo->data, alignment, numBuffers);
                offset = totalSize;
            }
        }
    }

    // if required allocate the buffer and reserve slots in it for the BufferInfo
    if (totalSize > 0)
    {
        auto buffer = vsg::Buffer::create(totalSize, bufferUsageFlags, VK_SHARING_MODE_EXCLUSIVE);
        for (auto& bufferInfo : bufferInfoList)
        {
            if (bufferInfo->data && !bufferInfo->buffer)
            {
                auto size = bufferSize(*bufferInfo->data, alignment, numBuffers);
                auto [allocated, offset] = buffer->reserve(size, alignment);
                if (allocated)
                {
                    bufferInfo->buffer = buffer;
                    bufferInfo->offset = offset;
                    bufferInfo->range = size;
                }
                else
                {
                    vsg::warn("BufferedDescriptorBuffer::compile(..) unable to allocate bufferInfo within associated Buffer.");
                }
            }
        }
    }

    // create the Vulkan objects for our buffer.
    Inherit::compile(context);

    // replace the BufferInfoList with a proxy list containing dynamic offsets and sub buffer range.
    vsg::BufferInfoList tmp_bufferInfoList;
    tmp_bufferInfoList.swap(bufferInfoList);
    for (auto& bi : tmp_bufferInfoList)
    {
        auto proxy_bufferInfo = vsg::BufferInfo::create(bi->buffer, bi->offset, bi->range/numBuffers, bi->data);
        // note, BufferInfo needs to have a parent to use dynamic offsets, otherwise the BufferInfo's destructor will release an incorrect buffer range on destruction.
        proxy_bufferInfo->parent = bi;
        bufferInfoList.push_back(proxy_bufferInfo);
    }
}
