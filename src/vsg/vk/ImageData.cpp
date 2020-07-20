/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/ImageData.h>
#include <vsg/io/Options.h>

#include <algorithm>
#include <iostream>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::transferImageData
//
ImageData vsg::transferImageData(Context& context, const Data* data, Sampler* sampler, VkImageLayout targetImageLayout)
{
    // std::cout<<"\nvsg::transferImageData()"<<std::endl;

    if (!data)
    {
        return ImageData(sampler, nullptr, targetImageLayout);
    }

    Device* device = context.device;

    VkDeviceSize imageTotalSize = data->dataSize();

    VkDeviceSize alignment = std::max(VkDeviceSize(4), VkDeviceSize(data->valueSize()));
    BufferData stagingBufferData = context.stagingMemoryBufferPools->reserveBufferData(imageTotalSize, alignment, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBufferData._data = const_cast<Data*>(data);

    //std::cout<<"stagingBufferData._buffer "<<stagingBufferData._buffer.get()<<", "<<stagingBufferData._offset<<", "<<stagingBufferData._range<<")"<<std::endl;

    ref_ptr<Buffer> imageStagingBuffer(stagingBufferData._buffer);
    ref_ptr<DeviceMemory> imageStagingMemory(imageStagingBuffer->getDeviceMemory());

    if (!imageStagingMemory)
    {
        return ImageData();
    }

    // copy image data to staging memory
    imageStagingMemory->copy(imageStagingBuffer->getMemoryOffset() + stagingBufferData._offset, imageTotalSize, data->dataPointer());

    uint32_t mipLevels = sampler != nullptr ? static_cast<uint32_t>(ceil(sampler->info().maxLod)) : 1;
    if (mipLevels == 0)
    {
        mipLevels = 1;
    }

    // clamp the mipLevels so that its no larger than what the data dimensions support
    uint32_t maxDimension = std::max({data->width(), data->height(), data->depth()});
    while ((1u << (mipLevels - 1)) > maxDimension)
    {
        --mipLevels;
    }

    //mipLevels = 1;  // disable mipmapping

    Data::Layout layout = data->getLayout();
    auto mipmapOffsets = data->computeMipmapOffsets();

    if (mipLevels > 1)
    {
        if (mipmapOffsets.size() > 1)
        {
            mipLevels = std::min(mipLevels, uint32_t(mipmapOffsets.size()));
        }
        else
        {
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(*(device->getPhysicalDevice()), layout.format, &formatProperties);

            if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0)
            {
                std::cout << "vsg::transferImageData(..) failed : formatProperties.optimalTilingFeatures VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT not supported, disabling mipmap generation." << std::endl;
                mipLevels = 1;
            }

            if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) == 0)
            {
                std::cout << "vsg::transferImageData(..) failed : formatProperties.optimalTilingFeatures VK_FORMAT_FEATURE_BLIT_DST_BIT not supported, disabling mipmap generation." << std::endl;
                mipLevels = 1;
            }
        }
    }

    bool generatMipmaps = (mipLevels > 1) && (mipmapOffsets.size() <= 1);

#if 0
    std::cout << "data->dataSize() = " << data->dataSize() << std::endl;
    std::cout << "data->width() = " << data->width() << std::endl;
    std::cout << "data->height() = " << data->height() << std::endl;
    std::cout << "data->depth() = " << data->depth() << std::endl;
    std::cout << "data->getLayout().format = " << data->getLayout().format << std::endl;
    std::cout << "sampler->info().maxLod = " << sampler->info().maxLod << std::endl;

    std::cout << "Creating imageStagingBuffer and memory size = " << imageTotalSize << " mipLevels = "<<mipLevels<<std::endl;
#endif

    // take the block dimensions into account for image size to allow for any block compressed image formats where the data dimensions is based in number of blocks so needs to be multiple to get final pixel count
    uint32_t width = data->width() * layout.blockWidth;
    uint32_t height = data->height() * layout.blockHeight;
    uint32_t depth = data->depth() * layout.blockDepth;

    auto dimensions = data->dimensions();
    VkImageType imageType = dimensions >= 3 ? VK_IMAGE_TYPE_3D : (dimensions == 2 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D);
    VkImageViewType imageViewType = dimensions >= 3 ? VK_IMAGE_VIEW_TYPE_3D : (dimensions == 2 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_1D);

    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = imageType;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = depth;
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = layout.format;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.pNext = nullptr;

    if (generatMipmaps)
    {
        imageCreateInfo.usage = imageCreateInfo.usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    ref_ptr<Image> textureImage = Image::create(device, imageCreateInfo);
    if (!textureImage)
    {
        return ImageData();
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(*device, *textureImage, &memRequirements);

    auto [deviceMemory, offset] = context.deviceMemoryBufferPools->reserveMemory(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (!deviceMemory)
    {
        std::cout << "Warning: vsg::transferImageData() Failed allocate memory to reserve slot" << std::endl;
        return ImageData();
    }

    textureImage->bind(deviceMemory, offset);

    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = *textureImage;
    createInfo.viewType = imageViewType;
    createInfo.format = layout.format;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = mipLevels;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    createInfo.pNext = nullptr;

    ref_ptr<ImageView> textureImageView = ImageView::create(device, createInfo);
    if (textureImageView) textureImageView->setImage(textureImage);

    ImageData imageData(sampler, textureImageView, targetImageLayout);

    context.copyImageDataCommands.emplace_back(new CopyAndReleaseImageDataCommand(stagingBufferData, imageData, mipLevels));

    return imageData;
}
