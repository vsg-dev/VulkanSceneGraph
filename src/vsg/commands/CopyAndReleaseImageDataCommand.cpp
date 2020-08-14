/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/CopyAndReleaseImageDataCommand.h>
#include <vsg/commands/PipelineBarrier.h>
#include <vsg/io/Options.h>

using namespace vsg;

CopyAndReleaseImageDataCommand::CopyAndReleaseImageDataCommand(BufferData src, ImageData dest)
{
    add(src, dest);
}

CopyAndReleaseImageDataCommand::CopyAndReleaseImageDataCommand(BufferData src, ImageData dest, uint32_t numMipMapLevels)
{
    add(src, dest, numMipMapLevels);
}

CopyAndReleaseImageDataCommand::~CopyAndReleaseImageDataCommand()
{
    for (auto& copyData : completed) copyData.source.release();
    for (auto& copyData : pending) copyData.source.release();
}

void CopyAndReleaseImageDataCommand::add(BufferData src, ImageData dest)
{
    pending.push_back(CopyData{src, dest, vsg::computeNumMipMapLevels(src.data, dest.sampler)});
}

void CopyAndReleaseImageDataCommand::add(BufferData src, ImageData dest, uint32_t numMipMapLevels)
{
    pending.push_back(CopyData{src, dest, numMipMapLevels});
}

void CopyAndReleaseImageDataCommand::CopyData::record(CommandBuffer& commandBuffer) const
{
    ref_ptr<Buffer> imageStagingBuffer(source.buffer);
    ref_ptr<Data> data(source.data);
    ref_ptr<Image> textureImage(destination.imageView->getImage());
    VkImageLayout targetImageLayout = destination.imageLayout;

    Data::Layout layout = data->getLayout();
    auto mipmapOffsets = data->computeMipmapOffsets();

    bool useDataMipmaps = (mipLevels > 1) && (mipmapOffsets.size() > 1);
    bool generatMipmaps = (mipLevels > 1) && (mipmapOffsets.size() <= 1);

    uint32_t width = data->width() * layout.blockWidth;
    uint32_t height = data->height() * layout.blockHeight;
    uint32_t depth = data->depth() * layout.blockDepth;

    auto vk_textureImage = textureImage->vk(commandBuffer.deviceID);

    // transfer the data.
    if (useDataMipmaps)
    {
        VkImageMemoryBarrier preCopyBarrier = {};
        preCopyBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        preCopyBarrier.srcAccessMask = 0;
        preCopyBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        preCopyBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        preCopyBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        preCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        preCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        preCopyBarrier.image = vk_textureImage;
        preCopyBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        preCopyBarrier.subresourceRange.baseArrayLayer = 0;
        preCopyBarrier.subresourceRange.layerCount = 1;
        preCopyBarrier.subresourceRange.levelCount = mipLevels;
        preCopyBarrier.subresourceRange.baseMipLevel = 0;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &preCopyBarrier);

        uint32_t mipWidth = width;
        uint32_t mipHeight = height;
        uint32_t mipDepth = depth;
        auto valueSize = data->valueSize();
        for (uint32_t mipLevel = 0; mipLevel < mipLevels; ++mipLevel)
        {
            // std::cout<<"   level = "<<mipLevel<<", mipWidth = "<<mipWidth<<", mipHeight = "<<mipHeight<<std::endl;
            VkBufferImageCopy region = {};
            region.bufferOffset = source.offset + mipmapOffsets[mipLevel] * valueSize;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = mipLevel;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {mipWidth, mipHeight, mipDepth};

            vkCmdCopyBufferToImage(commandBuffer, *imageStagingBuffer, vk_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
            if (mipDepth > 1) mipDepth /= 2;
        }

        VkImageMemoryBarrier postCopyBarrier = {};
        postCopyBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        postCopyBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        postCopyBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        postCopyBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        postCopyBarrier.newLayout = targetImageLayout;
        postCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        postCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        postCopyBarrier.image = vk_textureImage;
        postCopyBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        postCopyBarrier.subresourceRange.baseArrayLayer = 0;
        postCopyBarrier.subresourceRange.layerCount = 1;
        postCopyBarrier.subresourceRange.levelCount = mipLevels;
        postCopyBarrier.subresourceRange.baseMipLevel = 0;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &postCopyBarrier);
    }
    else if (generatMipmaps)
    {
        // generate mipmaps using Vulkan
        VkImageMemoryBarrier preCopyBarrier = {};
        preCopyBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        preCopyBarrier.srcAccessMask = 0;
        preCopyBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        preCopyBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        preCopyBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        preCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        preCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        preCopyBarrier.image = vk_textureImage;
        preCopyBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        preCopyBarrier.subresourceRange.baseArrayLayer = 0;
        preCopyBarrier.subresourceRange.layerCount = 1;
        preCopyBarrier.subresourceRange.levelCount = mipLevels;
        preCopyBarrier.subresourceRange.baseMipLevel = 0;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &preCopyBarrier);

        VkBufferImageCopy region = {};
        region.bufferOffset = source.offset;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, depth};

        vkCmdCopyBufferToImage(commandBuffer, *imageStagingBuffer, vk_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = vk_textureImage;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = width;
        int32_t mipHeight = height;
        int32_t mipDepth = depth;

        for (uint32_t i = 1; i < mipLevels; ++i)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            VkImageBlit blit = {};
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mipWidth, mipHeight, mipDepth};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, mipDepth > 1 ? mipDepth / 2 : 1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(commandBuffer,
                           vk_textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           vk_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &blit,
                           VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = targetImageLayout;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
            if (mipDepth > 1) mipDepth /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = targetImageLayout;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
    }
    else
    {

        auto preCopyImageMemoryBarrier = ImageMemoryBarrier::create(
            0, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            textureImage,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        auto preCopyPipelineBarrier = PipelineBarrier::create(
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, preCopyImageMemoryBarrier);

        preCopyPipelineBarrier->record(commandBuffer);

        VkBufferImageCopy region = {};
        region.bufferOffset = source.offset;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, depth};

        vkCmdCopyBufferToImage(commandBuffer, *imageStagingBuffer, vk_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        auto postCopyImageBarrier = ImageMemoryBarrier::create(
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, targetImageLayout,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            textureImage,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        auto postPipelineBarrier = PipelineBarrier::create(
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, postCopyImageBarrier);

        postPipelineBarrier->record(commandBuffer);
    }
}

void CopyAndReleaseImageDataCommand::record(CommandBuffer& commandBuffer) const
{
    for (auto& copyData : completed) copyData.source.release();
    completed.clear();

    for (auto& copyData : pending)
    {
        copyData.record(commandBuffer);
    }

    pending.swap(completed);
}
