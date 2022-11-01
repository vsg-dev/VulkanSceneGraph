/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/CopyAndReleaseImage.h>
#include <vsg/commands/PipelineBarrier.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

CopyAndReleaseImage::CopyAndReleaseImage(ref_ptr<MemoryBufferPools> optional_stagingMemoryBufferPools) :
    stagingMemoryBufferPools(optional_stagingMemoryBufferPools)
{
}

CopyAndReleaseImage::~CopyAndReleaseImage()
{
}

CopyAndReleaseImage::CopyData::CopyData(ref_ptr<BufferInfo> src, ref_ptr<ImageInfo> dest, uint32_t numMipMapLevels) :
    source(src),
    destination(dest),
    mipLevels(numMipMapLevels)
{
    if (source->data)
    {
        layout = source->data->properties;
        width = source->data->width();
        height = source->data->height();
        depth = source->data->depth();
        mipmapOffsets = source->data->computeMipmapOffsets();
    }
}

void CopyAndReleaseImage::add(const CopyData& cd)
{
    std::scoped_lock lock(_mutex);
    _pending.push_back(cd);
}

void CopyAndReleaseImage::add(ref_ptr<BufferInfo> src, ref_ptr<ImageInfo> dest)
{
    add(CopyData(src, dest, vsg::computeNumMipMapLevels(src->data, dest->sampler)));
}

void CopyAndReleaseImage::add(ref_ptr<BufferInfo> src, ref_ptr<ImageInfo> dest, uint32_t numMipMapLevels)
{
    add(CopyData(src, dest, numMipMapLevels));
}

void CopyAndReleaseImage::copy(ref_ptr<Data> data, ref_ptr<ImageInfo> dest)
{
    copy(data, dest, vsg::computeNumMipMapLevels(data, dest->sampler));
}

void CopyAndReleaseImage::copy(ref_ptr<Data> data, ref_ptr<ImageInfo> dest, uint32_t numMipMapLevels)
{
    if (!data) return;
    if (!stagingMemoryBufferPools) return;

    VkFormat sourceFormat = data->properties.format;
    VkFormat targetFormat = dest->imageView->format;

    if (sourceFormat == targetFormat)
    {
        _copyDirectly(data, dest, numMipMapLevels);
        return;
    }

    auto sourceTraits = getFormatTraits(sourceFormat);
    auto targetTraits = getFormatTraits(targetFormat);

    // assume data is compatible if sizes are consistent.
    bool formatsCompatible = sourceTraits.size == targetTraits.size;
    if (formatsCompatible)
    {
        _copyDirectly(data, dest, numMipMapLevels);
    }
    else
    {
        VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        VkDeviceSize imageTotalSize = targetTraits.size * data->valueCount();
        VkDeviceSize alignment = std::max(VkDeviceSize(4), VkDeviceSize(targetTraits.size));

        auto stagingBufferInfo = stagingMemoryBufferPools->reserveBuffer(imageTotalSize, alignment, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, memoryPropertyFlags);

        auto deviceID = stagingMemoryBufferPools->device->deviceID;
        ref_ptr<Buffer> imageStagingBuffer(stagingBufferInfo->buffer);
        ref_ptr<DeviceMemory> imageStagingMemory(imageStagingBuffer->getDeviceMemory(deviceID));

        if (!imageStagingMemory) return;

        vsg::CopyAndReleaseImage::CopyData cd(stagingBufferInfo, dest, numMipMapLevels);
        cd.width = data->width();
        cd.height = data->height();
        cd.depth = data->depth();
        cd.layout.format = targetFormat;
        cd.layout.stride = targetTraits.size;

        // set up a vec4 worth of default values for the type
        const uint8_t* default_ptr = targetTraits.defaultValue;
        uint32_t bytesFromSource = sourceTraits.size;
        uint32_t bytesToTarget = targetTraits.size;

        // copy data
        using value_type = uint8_t;
        const value_type* src_ptr = reinterpret_cast<const value_type*>(data->dataPointer());

        void* buffer_data;
        imageStagingMemory->map(imageStagingBuffer->getMemoryOffset(deviceID) + stagingBufferInfo->offset, imageTotalSize, 0, &buffer_data);
        value_type* dest_ptr = reinterpret_cast<value_type*>(buffer_data);

        size_t valueCount = data->valueCount();
        for (size_t i = 0; i < valueCount; ++i)
        {
            uint32_t s = 0;
            for (; s < bytesFromSource; ++s)
            {
                (*dest_ptr++) = *(src_ptr++);
            }

            const value_type* src_default = default_ptr;
            for (; s < bytesToTarget; ++s)
            {
                (*dest_ptr++) = *(src_default++);
            }
        }

        imageStagingMemory->unmap();

        add(cd);
    }
}

void CopyAndReleaseImage::_copyDirectly(ref_ptr<Data> data, ref_ptr<ImageInfo> dest, uint32_t numMipMapLevels)
{
    VkDeviceSize imageTotalSize = data->dataSize();
    VkDeviceSize alignment = std::max(VkDeviceSize(4), VkDeviceSize(data->valueSize()));

    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    auto stagingBufferInfo = stagingMemoryBufferPools->reserveBuffer(imageTotalSize, alignment, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, memoryPropertyFlags);
    stagingBufferInfo->data = data;

    auto deviceID = stagingMemoryBufferPools->device->deviceID;
    ref_ptr<Buffer> imageStagingBuffer(stagingBufferInfo->buffer);
    ref_ptr<DeviceMemory> imageStagingMemory(imageStagingBuffer->getDeviceMemory(deviceID));

    if (!imageStagingMemory) return;

    // copy data to staging memory
    imageStagingMemory->copy(imageStagingBuffer->getMemoryOffset(deviceID) + stagingBufferInfo->offset, imageTotalSize, data->dataPointer());

    add(stagingBufferInfo, dest, numMipMapLevels);
}

void CopyAndReleaseImage::CopyData::record(CommandBuffer& commandBuffer) const
{
    ref_ptr<Buffer> imageStagingBuffer(source->buffer);
    ref_ptr<Image> textureImage(destination->imageView->image);
    auto aspectMask = destination->imageView->subresourceRange.aspectMask;
    VkImageLayout targetImageLayout = destination->imageLayout;

    uint32_t faceWidth = width;
    uint32_t faceHeight = height;
    uint32_t faceDepth = depth;
    uint32_t arrayLayers = 1;

    //switch(layout.imageViewType)
    switch (destination->imageView->viewType)
    {
    case (VK_IMAGE_VIEW_TYPE_CUBE):
        arrayLayers = faceDepth;
        faceDepth = 1;
        break;
    case (VK_IMAGE_VIEW_TYPE_1D_ARRAY):
        arrayLayers = faceHeight * faceDepth;
        faceHeight = 1;
        faceDepth = 1;
        break;
    case (VK_IMAGE_VIEW_TYPE_2D_ARRAY):
        arrayLayers = faceDepth;
        faceDepth = 1;
        break;
    case (VK_IMAGE_VIEW_TYPE_CUBE_ARRAY):
        arrayLayers = faceDepth;
        faceDepth = 1;
        break;
    default:
        break;
    }

    uint32_t destWidth = faceWidth * layout.blockWidth;
    uint32_t destHeight = faceHeight * layout.blockHeight;
    uint32_t destDepth = faceDepth * layout.blockDepth;

    const auto valueSize = layout.stride; // data->valueSize();

    bool useDataMipmaps = (mipLevels > 1) && (mipmapOffsets.size() > 1);
    bool generatMipmaps = (mipLevels > 1) && (mipmapOffsets.size() <= 1);

    auto vk_textureImage = textureImage->vk(commandBuffer.deviceID);

    if (generatMipmaps)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(*(commandBuffer.getDevice()->getPhysicalDevice()), layout.format, &props);
        const bool isBlitPossible = (props.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) > 0;

        if (!isBlitPossible)
        {
            generatMipmaps = false;
        }
    }

    // transfer the data.
    VkImageMemoryBarrier preCopyBarrier = {};
    preCopyBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    preCopyBarrier.srcAccessMask = 0;
    preCopyBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    preCopyBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    preCopyBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    preCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    preCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    preCopyBarrier.image = vk_textureImage;
    preCopyBarrier.subresourceRange.aspectMask = aspectMask;
    preCopyBarrier.subresourceRange.baseArrayLayer = 0;
    preCopyBarrier.subresourceRange.layerCount = arrayLayers;
    preCopyBarrier.subresourceRange.levelCount = mipLevels;
    preCopyBarrier.subresourceRange.baseMipLevel = 0;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &preCopyBarrier);

    std::vector<VkBufferImageCopy> regions;

    if (useDataMipmaps)
    {
        size_t offset = 0u;
        regions.resize(mipLevels * arrayLayers);

        uint32_t mipWidth = destWidth;
        uint32_t mipHeight = destHeight;
        uint32_t mipDepth = destDepth;

        for (uint32_t mipLevel = 0; mipLevel < mipLevels; ++mipLevel)
        {
            const size_t faceSize = static_cast<size_t>(faceWidth * faceHeight * faceDepth * valueSize);

            for (uint32_t face = 0; face < arrayLayers; ++face)
            {
                auto& region = regions[mipLevel * arrayLayers + face];
                region.bufferOffset = source->offset + offset;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = aspectMask;
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
    }
    else
    {
        regions.resize(arrayLayers);

        const size_t faceSize = static_cast<size_t>(faceWidth * faceHeight * faceDepth * valueSize);
        for (auto face = 0u; face < arrayLayers; face++)
        {
            auto& region = regions[face];
            region.bufferOffset = source->offset + face * faceSize;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = aspectMask;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = face;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {destWidth, destHeight, destDepth};
        }
    }

    vkCmdCopyBufferToImage(commandBuffer, imageStagingBuffer->vk(commandBuffer.deviceID), vk_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(regions.size()), regions.data());

    if (generatMipmaps)
    {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = vk_textureImage;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = aspectMask;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = arrayLayers;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = destWidth;
        int32_t mipHeight = destHeight;
        int32_t mipDepth = destDepth;

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

            std::vector<VkImageBlit> blits(arrayLayers);

            for (auto face = 0u; face < arrayLayers; ++face)
            {
                auto& blit = blits[face];
                blit.srcOffsets[0] = {0, 0, 0};
                blit.srcOffsets[1] = {mipWidth, mipHeight, mipDepth};
                blit.srcSubresource.aspectMask = aspectMask;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = face;
                blit.srcSubresource.layerCount = arrayLayers;
                blit.dstOffsets[0] = {0, 0, 0};
                blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, mipDepth > 1 ? mipDepth / 2 : 1};
                blit.dstSubresource.aspectMask = aspectMask;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = face;
                blit.dstSubresource.layerCount = arrayLayers;
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
        VkImageMemoryBarrier postCopyBarrier = {};
        postCopyBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        postCopyBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        postCopyBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        postCopyBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        postCopyBarrier.newLayout = targetImageLayout;
        postCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        postCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        postCopyBarrier.image = vk_textureImage;
        postCopyBarrier.subresourceRange.aspectMask = aspectMask;
        postCopyBarrier.subresourceRange.baseArrayLayer = 0;
        postCopyBarrier.subresourceRange.layerCount = arrayLayers;
        postCopyBarrier.subresourceRange.levelCount = mipLevels;
        postCopyBarrier.subresourceRange.baseMipLevel = 0;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &postCopyBarrier);
    }
}

void CopyAndReleaseImage::record(CommandBuffer& commandBuffer) const
{
    std::scoped_lock lock(_mutex);

    _readyToClear.clear();

    _readyToClear.swap(_completed);

    for (auto& copyData : _pending)
    {
        copyData.record(commandBuffer);
    }

    _pending.swap(_completed);
}
