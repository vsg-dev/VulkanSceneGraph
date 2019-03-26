/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Descriptor.h>

#include <iostream>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::transferImageData
//
ImageData vsg::transferImageData(Context& context, const Data* data, Sampler* sampler)
{
    if (!data)
    {
        return ImageData();
    }

    Device* device = context.device;

    VkDeviceSize imageTotalSize = data->dataSize();

    ref_ptr<Buffer> imageStagingBuffer = Buffer::create(device, imageTotalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE);
    ref_ptr<DeviceMemory> imageStagingMemory = DeviceMemory::create(device, imageStagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    imageStagingBuffer->bind(imageStagingMemory, 0);

    // copy image data to staging memory
    imageStagingMemory->copy(0, imageTotalSize, data->dataPointer());

#if 0
    std::cout << "data->dataSize() = " << data->dataSize() << std::endl;
    std::cout << "data->width() = " << data->width() << std::endl;
    std::cout << "data->height() = " << data->height() << std::endl;
    std::cout << "data->depth() = " << data->depth() << std::endl;

    std::cout << "Creating imageStagingBuffer and memorory size = " << imageTotalSize << std::endl;
#endif

    VkImageType imageType = data->depth() > 1 ? VK_IMAGE_TYPE_3D : (data->width() > 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D);
    VkImageViewType imageViewType = data->depth() > 1 ? VK_IMAGE_VIEW_TYPE_3D : (data->width() > 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_1D);

    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = imageType;
    imageCreateInfo.extent.width = static_cast<uint32_t>(data->width());
    imageCreateInfo.extent.height = static_cast<uint32_t>(data->height());
    imageCreateInfo.extent.depth = static_cast<uint32_t>(data->depth());
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = data->getFormat();
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    ref_ptr<Image> textureImage = Image::create(device, imageCreateInfo);
    if (!textureImage)
    {
        return ImageData();
    }

#if 1
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(*device, *textureImage, &memRequirements);

    VkDeviceSize totalSize = memRequirements.size;

    ref_ptr<DeviceMemory> deviceMemory;
    DeviceMemory::OptionalMemoryOffset reservedSlot(false, 0);

    for(auto& memoryPool : context.memoryPools)
    {
        if (!memoryPool->full() && memoryPool->getMemoryRequirements().memoryTypeBits==memRequirements.memoryTypeBits)
        {
            reservedSlot = memoryPool->reserve(totalSize);
            if (reservedSlot.first)
            {
                deviceMemory = memoryPool;
                break;
            }
        }
    }

    if (!deviceMemory)
    {
        VkDeviceSize minumumDeviceMemorySize = context.minimumImageDeviceMemorySize;

        // clamp to an aligned size
        minumumDeviceMemorySize = ((minumumDeviceMemorySize+memRequirements.alignment-1)/memRequirements.alignment)*memRequirements.alignment;

        //std::cout<<"Creating new local DeviceMemory"<<std::endl;
        if (memRequirements.size<minumumDeviceMemorySize) memRequirements.size = minumumDeviceMemorySize;

        deviceMemory = vsg::DeviceMemory::create(device, memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (deviceMemory)
        {
            reservedSlot = deviceMemory->reserve(totalSize);
            if (!deviceMemory->full())
            {
                //std::cout<<"  inserting DeviceMemory into memoryPool "<<deviceMemory.get()<<std::endl;
                context.memoryPools.push_back(deviceMemory);
            }
        }
    }
    else
    {
        if (deviceMemory->full())
        {
            //std::cout<<"DeviceMemory is full "<<deviceMemory.get()<<std::endl;
        }
    }

    if (!reservedSlot.first)
    {
        std::cout<<"Failed to reserve slot"<<std::endl;
        return ImageData();
    }

    //std::cout<<"DeviceMemory "<<deviceMemory.get()<<" slot position = "<<reservedSlot.second<<", size = "<<totalSize<<std::endl;
    textureImage->bind(deviceMemory, reservedSlot.second);
#else

    ref_ptr<DeviceMemory> textureImageDeviceMemory = DeviceMemory::create(device, textureImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (!textureImageDeviceMemory)
    {
        return ImageData();
    }

    textureImage->bind(textureImageDeviceMemory, 0);
#endif

    dispatchCommandsToQueue(device, context.commandPool, context.graphicsQueue, [&](VkCommandBuffer commandBuffer) {
        ImageMemoryBarrier preCopyImageMemoryBarrier(
            0, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            textureImage);

        preCopyImageMemoryBarrier.cmdPiplineBarrier(commandBuffer,
                                                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {static_cast<uint32_t>(data->width()), static_cast<uint32_t>(data->height()), 1};

        vkCmdCopyBufferToImage(commandBuffer, *imageStagingBuffer, *textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        ImageMemoryBarrier postCopyImageMemoryBarrier(
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            textureImage);

        postCopyImageMemoryBarrier.cmdPiplineBarrier(commandBuffer,
                                                     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    });

    // clean up staging buffer
    imageStagingBuffer = 0;
    imageStagingMemory = 0;

    ref_ptr<Sampler> textureSampler = sampler != nullptr ? Sampler::Result(sampler) : Sampler::create(device);
    ref_ptr<ImageView> textureImageView = ImageView::create(device, textureImage, imageViewType, data->getFormat(), VK_IMAGE_ASPECT_COLOR_BIT);

    if (textureImageView) textureImageView->setImage(textureImage);

    return ImageData(textureSampler, textureImageView, VK_IMAGE_LAYOUT_UNDEFINED);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorBuffer
//
void DescriptorBuffer::copyDataListToBuffers()
{
    vsg::copyDataListToBuffers(_bufferDataList);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Texture
//
Texture::Texture() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
{
    // set default sampler info
    _samplerInfo = {};
    _samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    _samplerInfo.minFilter = VK_FILTER_LINEAR;
    _samplerInfo.magFilter = VK_FILTER_LINEAR;
    _samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    _samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    _samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
#if 1
    // requres Logical device to have deviceFeatures.samplerAnisotropy = VK_TRUE; set when creating the vsg::Device
    _samplerInfo.anisotropyEnable = VK_TRUE;
    _samplerInfo.maxAnisotropy = 16;
#else
    _samplerInfo.anisotropyEnable = VK_FALSE;
    _samplerInfo.maxAnisotropy = 1;
#endif
    _samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    _samplerInfo.unnormalizedCoordinates = VK_FALSE;
    _samplerInfo.compareEnable = VK_FALSE;
    _samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
}

void Texture::read(Input& input)
{
    Descriptor::read(input);

    _textureData = input.readObject<Data>("TextureData");
}

void Texture::write(Output& output) const
{
    Descriptor::write(output);

    output.writeObject("TextureData", _textureData.get());
}

void Texture::compile(Context& context)
{
    if (_implementation) return;

    ref_ptr<Sampler> sampler = Sampler::create(context.device, _samplerInfo, nullptr);
    vsg::ImageData imageData = vsg::transferImageData(context, _textureData, sampler);
    if (!imageData.valid())
    {
        return;
    }

    _implementation = vsg::DescriptorImage::create(_dstBinding, _dstArrayElement, _descriptorType, vsg::ImageDataList{imageData});
}

bool Texture::assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
{
    if (_implementation) return _implementation->assignTo(wds, descriptorSet);
    else return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Uniform
//
Uniform::Uniform() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
{
}

void Uniform::read(Input& input)
{
    Descriptor::read(input);

    _dataList.resize(input.readValue<uint32_t>("NumData"));
    for (auto& data : _dataList)
    {
        data = input.readObject<Data>("Data");
    }
}

void Uniform::write(Output& output) const
{
    Descriptor::write(output);

    output.writeValue<uint32_t>("NumData", _dataList.size());
    for (auto& data : _dataList)
    {
        output.writeObject("Data", data.get());
    }
}

void Uniform::compile(Context& context)
{
    if (_implementation) return;

    auto bufferDataList = vsg::createHostVisibleBuffer(context.device, _dataList, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    vsg::copyDataListToBuffers(bufferDataList);
    _implementation = vsg::DescriptorBuffer::create(_dstBinding, _dstArrayElement, _descriptorType, bufferDataList);
}

bool Uniform::assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
{
    if (_implementation) return _implementation->assignTo(wds, descriptorSet);
    else return false;
}

void Uniform::copyDataListToBuffers()
{
    if (_implementation) _implementation->copyDataListToBuffers();
}
