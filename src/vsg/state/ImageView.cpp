/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/state/ImageView.h>
#include <vsg/vk/Context.h>

using namespace vsg;

VkImageAspectFlags vsg::computeAspectFlagsForFormat(VkFormat format)
{
    if (format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT)
    {
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    else if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_X8_D24_UNORM_PACK32)
    {
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else
    {
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

void ImageView::VulkanData::release()
{
    if (imageView)
    {
        vkDestroyImageView(*device, imageView, device->getAllocationCallbacks());
        imageView = VK_NULL_HANDLE;
        device = {};
    }
}

ImageView::ImageView(ref_ptr<Image> in_image) :
    image(in_image)
{
    if (image)
    {
        if (image->data && image->data->properties.imageViewType >= 0)
        {
            viewType = static_cast<VkImageViewType>(image->data->properties.imageViewType);
        }
        else
        {
            auto imageType = image->imageType;
            viewType = (imageType == VK_IMAGE_TYPE_3D) ? VK_IMAGE_VIEW_TYPE_3D : ((imageType == VK_IMAGE_TYPE_2D) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_1D);
        }

        format = image->format;
        subresourceRange.aspectMask = computeAspectFlagsForFormat(image->format);
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = image->arrayLayers;
    }
}

ImageView::ImageView(ref_ptr<Image> in_image, VkImageAspectFlags aspectFlags) :
    image(in_image)
{
    if (image)
    {
        if (image->data && image->data->properties.imageViewType >= 0)
        {
            viewType = static_cast<VkImageViewType>(image->data->properties.imageViewType);
        }
        else
        {
            auto imageType = image->imageType;
            viewType = (imageType == VK_IMAGE_TYPE_3D) ? VK_IMAGE_VIEW_TYPE_3D : ((imageType == VK_IMAGE_TYPE_2D) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_1D);
        }

        format = image->format;
        subresourceRange.aspectMask = aspectFlags;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = image->arrayLayers;
    }
}

ImageView::~ImageView()
{
    for (auto& vd : _vulkanData) vd.release();
}

int ImageView::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(flags, rhs.flags))) return result;
    if ((result = compare_pointer(image, rhs.image))) return result;
    if ((result = compare_value(viewType, rhs.viewType))) return result;
    if ((result = compare_memory(components, rhs.components))) return result;
    return compare_memory(subresourceRange, rhs.subresourceRange);
}

void ImageView::compile(Device* device)
{
    auto& vd = _vulkanData[device->deviceID];
    if (vd.imageView != VK_NULL_HANDLE) return;

    vd.device = device;

    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    info.viewType = viewType;
    info.format = format;
    info.components = components;
    info.subresourceRange = subresourceRange;

    if (image)
    {
        image->compile(device);

        info.image = image->vk(device->deviceID);
    }

    if (VkResult result = vkCreateImageView(*vd.device, &info, vd.device->getAllocationCallbacks(), &vd.imageView); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create VkImageView.", result};
    }
}

void ImageView::compile(Context& context)
{
    auto& vd = _vulkanData[context.deviceID];
    if (vd.imageView != VK_NULL_HANDLE) return;

    vd.device = context.device;

    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    info.viewType = viewType;
    info.format = format;
    info.components = components;
    info.subresourceRange = subresourceRange;

    if (image)
    {
        image->compile(context);

        info.image = image->vk(vd.device->deviceID);
    }

    if (VkResult result = vkCreateImageView(*vd.device, &info, vd.device->getAllocationCallbacks(), &vd.imageView); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create VkImageView.", result};
    }
}

ref_ptr<ImageView> vsg::createImageView(vsg::Context& context, ref_ptr<Image> image, VkImageAspectFlags aspectFlags)
{
    vsg::Device* device = context.device;

    image->compile(device);

    // get memory requirements
    VkMemoryRequirements memRequirements = image->getMemoryRequirements(device->deviceID);

    // allocate memory with out export memory info extension
    auto [deviceMemory, offset] = context.deviceMemoryBufferPools->reserveMemory(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    image->bind(deviceMemory, offset);

    auto imageView = ImageView::create(image, aspectFlags);
    imageView->compile(device);

    return imageView;
}

ref_ptr<ImageView> vsg::createImageView(Device* device, ref_ptr<Image> image, VkImageAspectFlags aspectFlags)
{
    image->compile(device);

    image->allocateAndBindMemory(device);

    auto imageView = ImageView::create(image, aspectFlags);
    imageView->compile(device);

    return imageView;
}

void vsg::transferImageData(ref_ptr<ImageView> imageView, VkImageLayout targetImageLayout, Data::Properties properties, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, const Data::MipmapOffsets& mipmapOffsets, ref_ptr<Buffer> stagingBuffer, VkDeviceSize stagingBufferOffset, VkCommandBuffer commandBuffer, vsg::Device* device)
{
    ref_ptr<Image> textureImage(imageView->image);
    auto aspectMask = imageView->subresourceRange.aspectMask;

    uint32_t faceWidth = width;
    uint32_t faceHeight = height;
    uint32_t faceDepth = depth;
    uint32_t arrayLayers = 1;

    //switch(properties.imageViewType)
    switch (imageView->viewType)
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

    uint32_t destWidth = faceWidth * properties.blockWidth;
    uint32_t destHeight = faceHeight * properties.blockHeight;
    uint32_t destDepth = faceDepth * properties.blockDepth;

    const auto valueSize = properties.stride; // data->valueSize();

    bool useDataMipmaps = (mipLevels > 1) && (mipmapOffsets.size() > 1);
    bool generateMipmaps = (mipLevels > 1) && (mipmapOffsets.size() <= 1);

    auto vk_textureImage = textureImage->vk(device->deviceID);

    if (generateMipmaps)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(*(device->getPhysicalDevice()), properties.format, &props);
        const bool isBlitPossible = (props.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) > 0;

        if (!isBlitPossible)
        {
            generateMipmaps = false;
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
                region.bufferOffset = stagingBufferOffset + offset;
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
            region.bufferOffset = stagingBufferOffset + face * faceSize;
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

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer->vk(device->deviceID), vk_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(regions.size()), regions.data());

    if (generateMipmaps)
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

            VkImageBlit blit;
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mipWidth, mipHeight, mipDepth};
            blit.srcSubresource.aspectMask = aspectMask;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = arrayLayers;
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, mipDepth > 1 ? mipDepth / 2 : 1};
            blit.dstSubresource.aspectMask = aspectMask;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = arrayLayers;

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
