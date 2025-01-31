/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/CopyAndReleaseImage.h>
#include <vsg/commands/PipelineBarrier.h>
#include <vsg/io/Logger.h>
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
    transferImageData(destination->imageView, destination->imageLayout, layout, width, height, depth, mipLevels, mipmapOffsets, source->buffer, source->offset, commandBuffer.vk(), commandBuffer.getDevice());
}

void CopyAndReleaseImage::record(CommandBuffer& commandBuffer) const
{
    std::scoped_lock lock(_mutex);

    _readyToClear.clear();

    _readyToClear.swap(_completed);

    for (const auto& copyData : _pending)
    {
        copyData.record(commandBuffer);
    }

    _pending.swap(_completed);
}
