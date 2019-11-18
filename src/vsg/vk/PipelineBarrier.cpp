/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/PipelineBarrier.h>

using namespace vsg;

void MemoryBarrier::assign(VkMemoryBarrier& info) const
{
    info.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    info.pNext = nullptr;
    info.srcAccessMask = srcAccessMask;
    info.dstAccessMask = dstAccessMask;
}

void BufferMemoryBarrier::assign(VkBufferMemoryBarrier& info) const
{
    info.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    info.pNext = nullptr;
    info.srcAccessMask = srcAccessMask;
    info.dstAccessMask = dstAccessMask;
    info.srcQueueFamilyIndex = srcQueueFamilyIndex; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
    info.dstQueueFamilyIndex = dstQueueFamilyIndex; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
    info.buffer = buffer.valid() ? VkBuffer(*buffer) : VK_NULL_HANDLE;
    info.offset = offset;
    info.size = size;
}

void ImageMemoryBarrier::assign(VkImageMemoryBarrier& info) const
{
    info.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    info.pNext = nullptr;
    info.srcAccessMask = srcAccessMask;
    info.dstAccessMask = dstAccessMask;
    info.oldLayout = oldLayout;
    info.newLayout = newLayout;
    info.srcQueueFamilyIndex = srcQueueFamilyIndex; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
    info.dstQueueFamilyIndex = dstQueueFamilyIndex; // Queue::queueFamilyIndex() or VK_QUEUE_FAMILY_IGNORED
    info.image = image.valid() ? VkImage(*image) : VK_NULL_HANDLE;
    info.subresourceRange = subresourceRange;
}

void SampleLocations::apply(VkSampleLocationsInfoEXT& info) const
{
    info.sType = VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT;
    info.pNext = nullptr;
    info.sampleLocationsPerPixel = sampleLocationsPerPixel;
    info.sampleLocationGridSize = sampleLocationGridSize;
    info.sampleLocationsCount = static_cast<uint32_t>(sampleLocations.size());
    info.pSampleLocations = reinterpret_cast<const VkSampleLocationEXT*>(sampleLocations.data());
}

PipelineBarrier::PipelineBarrier()
{
}

PipelineBarrier::~PipelineBarrier()
{
}

void PipelineBarrier::dispatch(CommandBuffer& commandBuffer) const
{
    std::vector<VkMemoryBarrier> vk_memoryBarriers(memoryBarriers.size());
    for (size_t i = 0; i < memoryBarriers.size(); ++i)
    {
        memoryBarriers[i]->assign(vk_memoryBarriers[i]);
    }

    std::vector<VkBufferMemoryBarrier> vk_bufferMemoryBarriers(bufferMemoryBarriers.size());
    for (size_t i = 0; i < bufferMemoryBarriers.size(); ++i)
    {
        bufferMemoryBarriers[i]->assign(vk_bufferMemoryBarriers[i]);
    }

    std::vector<VkImageMemoryBarrier> vk_imageMemoryBarriers(imageMemoryBarriers.size());
    for (size_t i = 0; i < imageMemoryBarriers.size(); ++i)
    {
        imageMemoryBarriers[i]->assign(vk_imageMemoryBarriers[i]);
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStageMask,
        dstStageMask,
        dependencyFlags,
        static_cast<uint32_t>(vk_memoryBarriers.size()),
        vk_memoryBarriers.data(),
        static_cast<uint32_t>(vk_bufferMemoryBarriers.size()),
        vk_bufferMemoryBarriers.data(),
        static_cast<uint32_t>(vk_imageMemoryBarriers.size()),
        vk_imageMemoryBarriers.data());
}
