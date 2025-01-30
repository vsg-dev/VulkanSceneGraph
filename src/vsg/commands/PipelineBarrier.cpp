/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/PipelineBarrier.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// MemoryBarrier
//
void MemoryBarrier::assign(CommandBuffer& commandBuffer, VkMemoryBarrier& info) const
{
    info.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    info.pNext = next ? next->assign(commandBuffer) : nullptr;
    info.srcAccessMask = srcAccessMask;
    info.dstAccessMask = dstAccessMask;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// BufferMemoryBarrier
//
void BufferMemoryBarrier::assign(CommandBuffer& commandBuffer, VkBufferMemoryBarrier& info) const
{
    info.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    info.pNext = next ? next->assign(commandBuffer) : nullptr;
    info.srcAccessMask = srcAccessMask;
    info.dstAccessMask = dstAccessMask;
    info.srcQueueFamilyIndex = srcQueueFamilyIndex; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
    info.dstQueueFamilyIndex = dstQueueFamilyIndex; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
    info.buffer = buffer.valid() ? buffer->vk(commandBuffer.deviceID) : VK_NULL_HANDLE;
    info.offset = offset;
    info.size = size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// ImageMemoryBarrier
//
void ImageMemoryBarrier::assign(CommandBuffer& commandBuffer, VkImageMemoryBarrier& info) const
{
    info.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    info.pNext = next ? next->assign(commandBuffer) : nullptr;
    info.srcAccessMask = srcAccessMask;
    info.dstAccessMask = dstAccessMask;
    info.oldLayout = oldLayout;
    info.newLayout = newLayout;
    info.srcQueueFamilyIndex = srcQueueFamilyIndex; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
    info.dstQueueFamilyIndex = dstQueueFamilyIndex; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
    info.image = image.valid() ? image->vk(commandBuffer.deviceID) : VK_NULL_HANDLE;
    info.subresourceRange = subresourceRange;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// SampleLocations
//
void* SampleLocations::assign(CommandBuffer& commandBuffer) const
{
    auto info = commandBuffer.scratchMemory->allocate<VkSampleLocationsInfoEXT>(1);

    info->sType = VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT;
    info->pNext = next ? next->assign(commandBuffer) : nullptr;
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

void PipelineBarrier::record(CommandBuffer& commandBuffer) const
{
    auto& scratchMemory = *(commandBuffer.scratchMemory);

    auto vk_memoryBarriers = scratchMemory.allocate<VkMemoryBarrier>(memoryBarriers.size());
    for (size_t i = 0; i < memoryBarriers.size(); ++i)
    {
        memoryBarriers[i]->assign(commandBuffer, vk_memoryBarriers[i]);
    }

    auto vk_bufferMemoryBarriers = scratchMemory.allocate<VkBufferMemoryBarrier>(bufferMemoryBarriers.size());
    for (size_t i = 0; i < bufferMemoryBarriers.size(); ++i)
    {
        bufferMemoryBarriers[i]->assign(commandBuffer, vk_bufferMemoryBarriers[i]);
    }
    auto vk_imageMemoryBarriers = scratchMemory.allocate<VkImageMemoryBarrier>(imageMemoryBarriers.size());
    for (size_t i = 0; i < imageMemoryBarriers.size(); ++i)
    {
        imageMemoryBarriers[i]->assign(commandBuffer, vk_imageMemoryBarriers[i]);
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStageMask,
        dstStageMask,
        dependencyFlags,
        static_cast<uint32_t>(memoryBarriers.size()),
        vk_memoryBarriers,
        static_cast<uint32_t>(bufferMemoryBarriers.size()),
        vk_bufferMemoryBarriers,
        static_cast<uint32_t>(imageMemoryBarriers.size()),
        vk_imageMemoryBarriers);

    scratchMemory.release();
}
