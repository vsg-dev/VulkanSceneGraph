/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/CopyAndReleaseImage.h>
#include <vsg/commands/PipelineBarrier.h>
#include <vsg/io/Options.h>

using namespace vsg;

CopyAndReleaseImage::CopyAndReleaseImage(BufferInfo src, ImageInfo dest)
{
    add(src, dest);
}

CopyAndReleaseImage::CopyAndReleaseImage(BufferInfo src, ImageInfo dest, uint32_t numMipMapLevels)
{
    add(src, dest, numMipMapLevels);
}

CopyAndReleaseImage::~CopyAndReleaseImage()
{
    for (auto& copyData : completed) copyData.source.release();
    for (auto& copyData : pending) copyData.source.release();
}

void CopyAndReleaseImage::add(BufferInfo src, ImageInfo dest)
{
    pending.push_back(CopyData{src, dest, vsg::computeNumMipMapLevels(src.data, dest.sampler)});
}

void CopyAndReleaseImage::add(BufferInfo src, ImageInfo dest, uint32_t numMipMapLevels)
{
    pending.push_back(CopyData{src, dest, numMipMapLevels});
}

void CopyAndReleaseImage::CopyData::record(CommandBuffer& commandBuffer) const
{
    ref_ptr<Buffer> imageStagingBuffer(source.buffer);
    ref_ptr<Data> data(source.data);
    ref_ptr<Image> textureImage(destination.imageView->image);
    VkImageLayout targetImageLayout = destination.imageLayout;

    Data::Layout layout = data->getLayout();
    auto mipmapOffsets = data->computeMipmapOffsets();

    bool useDataMipmaps = (mipLevels > 1) && (mipmapOffsets.size() > 1);
    bool generatMipmaps = (mipLevels > 1) && (mipmapOffsets.size() <= 1);
    bool isCubemap = data->depth() == 6 && destination.imageView->viewType == VK_IMAGE_VIEW_TYPE_CUBE;

    uint32_t width = data->width() * layout.blockWidth;
    uint32_t height = data->height() * layout.blockHeight;
    uint32_t depth = isCubemap ? 1 : data->depth() * layout.blockDepth;
    uint32_t faceWidth = data->width();
    uint32_t faceHeight = data->height();
    uint32_t faceDepth = isCubemap ? 1 : data->depth();
    const uint32_t numFaces = isCubemap ? 6u : 1u;
    size_t offset = 0u;
    const auto valueSize = data->valueSize();

    auto vk_textureImage = textureImage->vk(commandBuffer.deviceID);

    VkFormatProperties props;
    const auto physicalDevice = commandBuffer.getDevice()->getPhysicalDevice()->getPhysicalDevice();
    vkGetPhysicalDeviceFormatProperties(physicalDevice, layout.format, &props);
    const bool isBlitPossible = (props.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) > 0;

    if (!isBlitPossible && generatMipmaps)
    {
        generatMipmaps = false;
        //std::cerr << "Can not create mipmap chain for format: " << layout.format << std::endl;
    }

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
        preCopyBarrier.subresourceRange.layerCount = numFaces;
        preCopyBarrier.subresourceRange.levelCount = mipLevels;
        preCopyBarrier.subresourceRange.baseMipLevel = 0;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &preCopyBarrier);

        std::vector<VkBufferImageCopy> regions(mipLevels * numFaces);

        uint32_t mipWidth = width;
        uint32_t mipHeight = height;
        uint32_t mipDepth = depth;

        for (uint32_t mipLevel = 0; mipLevel < mipLevels; ++mipLevel)
        {
            // std::cout<<"   level = "<<mipLevel<<", mipWidth = "<<mipWidth<<", mipHeight = "<<mipHeight<<std::endl;
            const size_t faceSize = static_cast<size_t>(faceWidth * faceHeight * faceDepth * valueSize);

            for (uint32_t face = 0; face < numFaces; ++face)
            {
                auto& region = regions[mipLevel * numFaces + face];
                region.bufferOffset = source.offset + offset;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = mipLevel;
                region.imageSubresource.baseArrayLayer = face;
                region.imageSubresource.layerCount = 1;
                region.imageOffset = {0, 0, 0};
                region.imageExtent = {mipWidth, mipHeight, mipDepth};

                offset += faceSize;
            }

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
            if (mipDepth > 1) mipDepth /= 2;
            if (faceWidth > 1) faceWidth /= 2;
            if (faceHeight > 1) faceHeight /= 2;
            if (faceDepth > 1) faceDepth /= 2;
        }

        vkCmdCopyBufferToImage(commandBuffer, imageStagingBuffer->vk(commandBuffer.deviceID), vk_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               static_cast<uint32_t>(regions.size()), regions.data());

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
        postCopyBarrier.subresourceRange.layerCount = numFaces;
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
        preCopyBarrier.subresourceRange.layerCount = numFaces;
        preCopyBarrier.subresourceRange.levelCount = mipLevels;
        preCopyBarrier.subresourceRange.baseMipLevel = 0;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &preCopyBarrier);

        for (auto face = 0u; face < numFaces; ++face)
        {
            VkBufferImageCopy region = {};
            region.bufferOffset = source.offset + offset;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = face;
            region.imageSubresource.layerCount = numFaces;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {width, height, depth};

            offset += static_cast<size_t>(faceWidth * faceHeight * faceDepth * valueSize);

            vkCmdCopyBufferToImage(commandBuffer, imageStagingBuffer->vk(commandBuffer.deviceID), vk_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        }

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = vk_textureImage;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = numFaces;
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

            std::vector<VkImageBlit> blits(numFaces);

            for (auto face = 0u; face < numFaces; ++face)
            {
                auto& blit = blits[face];
                blit.srcOffsets[0] = {0, 0, 0};
                blit.srcOffsets[1] = {mipWidth, mipHeight, mipDepth};
                blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = face;
                blit.srcSubresource.layerCount = numFaces;
                blit.dstOffsets[0] = {0, 0, 0};
                blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, mipDepth > 1 ? mipDepth / 2 : 1};
                blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = face;
                blit.dstSubresource.layerCount = numFaces;
            }

            vkCmdBlitImage(commandBuffer,
                           vk_textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           vk_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(blits.size()), blits.data(),
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
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, numFaces});

        auto preCopyPipelineBarrier = PipelineBarrier::create(
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, preCopyImageMemoryBarrier);

        preCopyPipelineBarrier->record(commandBuffer);

        const size_t faceSize = static_cast<size_t>(faceWidth * faceHeight * faceDepth * valueSize);
        std::vector<VkBufferImageCopy> regions(numFaces);

        for (auto face = 0u; face < numFaces; face++)
        {
            auto& region = regions[face];
            region.bufferOffset = source.offset + face * faceSize;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = face;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {width, height, depth};
        }

        vkCmdCopyBufferToImage(commandBuffer, imageStagingBuffer->vk(commandBuffer.deviceID), vk_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               static_cast<uint32_t>(regions.size()), regions.data());

        auto postCopyImageBarrier = ImageMemoryBarrier::create(
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, targetImageLayout,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            textureImage,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, numFaces});

        auto postPipelineBarrier = PipelineBarrier::create(
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, postCopyImageBarrier);

        postPipelineBarrier->record(commandBuffer);
    }
}

void CopyAndReleaseImage::record(CommandBuffer& commandBuffer) const
{
    for (auto& copyData : completed) copyData.source.release();
    completed.clear();

    for (auto& copyData : pending)
    {
        copyData.record(commandBuffer);
    }

    pending.swap(completed);
}
