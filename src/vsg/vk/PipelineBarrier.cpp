/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/PipelineBarrier.h>

using namespace vsg;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// ScratchBuffer
//
ScratchBuffer::ScratchBuffer(uint32_t size)
{
    buffer_begin = new uint8_t[size];
    buffer_end = buffer_begin + size;
    base_ptr = buffer_begin;
    requiresDelete = true;
}

ScratchBuffer::ScratchBuffer(const ScratchBuffer& parent, uint32_t minimumSize)
{
    if ((parent.buffer_end - parent.buffer_begin) >= minimumSize)
    {
        buffer_begin = parent.buffer_begin;
        buffer_end = parent.buffer_end;
        base_ptr = buffer_begin;
        requiresDelete = false;
    }
    else
    {
        buffer_begin = new uint8_t[minimumSize];
        buffer_end = buffer_begin + minimumSize;
        base_ptr = buffer_begin;
        requiresDelete = true;
    }
}

ScratchBuffer::~ScratchBuffer()
{
    if (requiresDelete) delete[] buffer_begin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// MemoryBarrier
//
void MemoryBarrier::assign(VkMemoryBarrier& info, ScratchBuffer& scratchBuffer) const
{
    info.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    info.pNext = next ? next->assign(scratchBuffer) : nullptr;
    info.srcAccessMask = srcAccessMask;
    info.dstAccessMask = dstAccessMask;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// BufferMemoryBarrier
//
void BufferMemoryBarrier::assign(VkBufferMemoryBarrier& info, ScratchBuffer& scratchBuffer) const
{
    info.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    info.pNext = next ? next->assign(scratchBuffer) : nullptr;
    info.srcAccessMask = srcAccessMask;
    info.dstAccessMask = dstAccessMask;
    info.srcQueueFamilyIndex = srcQueueFamilyIndex; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
    info.dstQueueFamilyIndex = dstQueueFamilyIndex; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
    info.buffer = buffer.valid() ? VkBuffer(*buffer) : VK_NULL_HANDLE;
    info.offset = offset;
    info.size = size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// ImageMemoryBarrier
//
void ImageMemoryBarrier::assign(VkImageMemoryBarrier& info, ScratchBuffer& scratchBuffer) const
{
    info.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    info.pNext = next ? next->assign(scratchBuffer) : nullptr;
    info.srcAccessMask = srcAccessMask;
    info.dstAccessMask = dstAccessMask;
    info.oldLayout = oldLayout;
    info.newLayout = newLayout;
    info.srcQueueFamilyIndex = srcQueueFamilyIndex; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
    info.dstQueueFamilyIndex = dstQueueFamilyIndex; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
    info.image = image.valid() ? VkImage(*image) : VK_NULL_HANDLE;
    info.subresourceRange = subresourceRange;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// SampleLocations
//
void* SampleLocations::assign(ScratchBuffer& scratchBuffer) const
{
    auto info = scratchBuffer.allocate<VkSampleLocationsInfoEXT>(1);

    info->sType = VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT;
    info->pNext = next ? next->assign(scratchBuffer) : nullptr;
    info->sampleLocationsPerPixel = sampleLocationsPerPixel;
    info->sampleLocationGridSize = sampleLocationGridSize;
    info->sampleLocationsCount = static_cast<uint32_t>(sampleLocations.size());
    info->pSampleLocations = reinterpret_cast<const VkSampleLocationEXT*>(sampleLocations.data());

    return info;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// PipelineBarrier
//
PipelineBarrier::PipelineBarrier()
{
}

PipelineBarrier::~PipelineBarrier()
{
}

void PipelineBarrier::dispatch(CommandBuffer& commandBuffer) const
{
    uint32_t total_size = 0;
    for (auto& mb : memoryBarriers) total_size += mb->infoSize();
    for (auto& bmb : bufferMemoryBarriers) total_size += bmb->infoSize();
    for (auto& imb : imageMemoryBarriers) total_size += imb->infoSize();

    ScratchBuffer scratchBuffer(total_size);

    auto* vk_memoryBarriers = scratchBuffer.allocate<VkMemoryBarrier>(memoryBarriers.size());
    for (size_t i = 0; i < memoryBarriers.size(); ++i)
    {
        memoryBarriers[i]->assign(vk_memoryBarriers[i], scratchBuffer);
    }

    auto vk_bufferMemoryBarriers = scratchBuffer.allocate<VkBufferMemoryBarrier>(bufferMemoryBarriers.size());
    for (size_t i = 0; i < bufferMemoryBarriers.size(); ++i)
    {
        bufferMemoryBarriers[i]->assign(vk_bufferMemoryBarriers[i], scratchBuffer);
    }
    auto vk_imageMemoryBarriers = scratchBuffer.allocate<VkImageMemoryBarrier>(imageMemoryBarriers.size());
    for (size_t i = 0; i < imageMemoryBarriers.size(); ++i)
    {
        imageMemoryBarriers[i]->assign(vk_imageMemoryBarriers[i], scratchBuffer);
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStageMask,
        dstStageMask,
        dependencyFlags,
        memoryBarriers.size(),
        vk_memoryBarriers,
        bufferMemoryBarriers.size(),
        vk_bufferMemoryBarriers,
        imageMemoryBarriers.size(),
        vk_imageMemoryBarriers);
}
