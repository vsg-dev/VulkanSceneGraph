/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/TransferTask.h>
#include <vsg/app/View.h>
#include <vsg/core/MipmapLayout.h>
#include <vsg/io/Logger.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/utils/Instrumentation.h>
#include <vsg/vk/State.h>

using namespace vsg;

TransferTask::TransferTask(Device* in_device, uint32_t numBuffers) :
    device(in_device),
    _bufferCount(numBuffers)
{
    CPU_INSTRUMENTATION_L1(instrumentation);

    _earlyDataToCopy.name = "_earlyDataToCopy";
    _lateDataToCopy.name = "_lateDataToCopy";

    for (uint32_t i = 0; i < numBuffers; ++i)
    {
        _earlyDataToCopy.frames.emplace_back(TransferBlock::create());
        _lateDataToCopy.frames.emplace_back(TransferBlock::create());
    }

    // level = Logger::LOGGER_INFO;
}

bool TransferTask::containsDataToTransfer(TransferMask transferMask) const
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return (((transferMask & TRANSFER_BEFORE_RECORD_TRAVERSAL) != 0) && _earlyDataToCopy.containsDataToTransfer()) ||
           (((transferMask & TRANSFER_AFTER_RECORD_TRAVERSAL) != 0) && _lateDataToCopy.containsDataToTransfer());
}

void TransferTask::assignTransferConsumedCompletedSemaphore(TransferMask transferMask, ref_ptr<Semaphore> semaphore)
{
    if ((transferMask & TRANSFER_BEFORE_RECORD_TRAVERSAL) != 0) _earlyDataToCopy.transferConsumerCompletedSemaphore = semaphore;
    if ((transferMask & TRANSFER_AFTER_RECORD_TRAVERSAL) != 0) _lateDataToCopy.transferConsumerCompletedSemaphore = semaphore;
}

void TransferTask::assign(const ResourceRequirements::DynamicData& dynamicData)
{
    CPU_INSTRUMENTATION_L2(instrumentation);

    assign(dynamicData.bufferInfos);
    assign(dynamicData.imageInfos);
}

void TransferTask::assign(const BufferInfoList& bufferInfoList)
{
    CPU_INSTRUMENTATION_L2(instrumentation);

    std::scoped_lock<std::mutex> lock(_mutex);

    log(level, "TransferTask::assign(BufferInfoList) ", this, ", bufferInfoList.size() = ", bufferInfoList.size());

    for (auto& bufferInfo : bufferInfoList)
    {
        if (bufferInfo->buffer && bufferInfo->data)
        {
            DataToCopy& dataToCopy = (bufferInfo->data->properties.dataVariance >= DYNAMIC_DATA_TRANSFER_AFTER_RECORD) ? _lateDataToCopy : _earlyDataToCopy;
            dataToCopy.dataMap[bufferInfo->buffer][bufferInfo->offset] = bufferInfo;
        }
        else
        {
            debug("TransferTask::assign(const BufferInfoList& bufferInfoList) bufferInfo ignored as incomplete, buffer = ", bufferInfo->buffer, ", data = ", bufferInfo->data);
        }
    }
}

bool TransferTask::DataToCopy::requiresCopy(uint32_t deviceID) const
{
    for (auto buffer_itr = dataMap.begin(); buffer_itr != dataMap.end(); ++buffer_itr)
    {
        auto& bufferInfos = buffer_itr->second;
        for (auto bufferInfo_itr = bufferInfos.begin(); bufferInfo_itr != bufferInfos.end(); ++bufferInfo_itr)
        {
            auto& bufferInfo = bufferInfo_itr->second;
            if ((bufferInfo->referenceCount() > 1) && bufferInfo->requiresCopy(deviceID)) return true;
        }
    }

    for (auto imageInfo_itr = imageInfoSet.begin(); imageInfo_itr != imageInfoSet.end(); ++imageInfo_itr)
    {
        auto& imageInfo = *imageInfo_itr;
        if ((imageInfo->referenceCount() > 1) && imageInfo->requiresCopy(deviceID)) return true;
    }

    return false;
}

void TransferTask::_transferBufferInfos(DataToCopy& dataToCopy, VkCommandBuffer vk_commandBuffer, TransferBlock& frame, VkDeviceSize& offset)
{
    CPU_INSTRUMENTATION_L1(instrumentation);

    auto deviceID = device->deviceID;
    auto& staging = frame.staging;
    auto& copyRegions = frame.copyRegions;
    auto& buffer_data = frame.buffer_data;

    VkDeviceSize alignment = 4;

    copyRegions.clear();
    copyRegions.resize(dataToCopy.dataTotalRegions);
    VkBufferCopy* pRegions = copyRegions.data();

    log(level, "  TransferTask::_transferBufferInfos(..) ", this);

    // copy any modified BufferInfo
    for (auto buffer_itr = dataToCopy.dataMap.begin(); buffer_itr != dataToCopy.dataMap.end();)
    {
        auto& bufferInfos = buffer_itr->second;

        uint32_t regionCount = 0;
        log(level, "    copying bufferInfos.size() = ", bufferInfos.size(), "{");
        for (auto bufferInfo_itr = bufferInfos.begin(); bufferInfo_itr != bufferInfos.end();)
        {
            auto& bufferInfo = bufferInfo_itr->second;
            if (bufferInfo->referenceCount() == 1)
            {
                log(level, "    BufferInfo only ref left ", bufferInfo, ", ", bufferInfo->referenceCount());
                bufferInfo_itr = bufferInfos.erase(bufferInfo_itr);
            }
            else
            {
                if (bufferInfo->syncModifiedCounts(deviceID))
                {
                    // copy data to staging buffer memory
                    char* ptr = reinterpret_cast<char*>(buffer_data) + offset;
                    std::memcpy(ptr, bufferInfo->data->dataPointer(), bufferInfo->range);

                    // record region
                    pRegions[regionCount++] = VkBufferCopy{offset, bufferInfo->offset, bufferInfo->range};

                    log(level, "       copying ", bufferInfo, ", ", bufferInfo->data, " to ", static_cast<void*>(ptr));

                    VkDeviceSize endOfEntry = offset + bufferInfo->range;
                    offset = (/*alignment == 1 ||*/ (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
                }
                else
                {
                    log(level, "       no need to copy ", bufferInfo);
                }

                if (bufferInfo->data->dynamic())
                {
                    ++bufferInfo_itr;
                }
                else
                {
                    log(level, "       removing copied static data: ", bufferInfo, ", ", bufferInfo->data);
                    bufferInfo_itr = bufferInfos.erase(bufferInfo_itr);
                }
            }
        }
        log(level, "    } bufferInfos.size() = ", bufferInfos.size(), "{");

        if (regionCount > 0)
        {
            auto& buffer = buffer_itr->first;

            vkCmdCopyBuffer(vk_commandBuffer, staging->vk(deviceID), buffer->vk(deviceID), regionCount, pRegions);

            log(level, "   vkCmdCopyBuffer(", ", ", staging->vk(deviceID), ", ", buffer->vk(deviceID), ", ", regionCount, ", ", pRegions);

            // advance to next buffer
            pRegions += regionCount;
        }

        if (bufferInfos.empty())
        {
            log(level, "    bufferInfos.empty()");
            buffer_itr = dataToCopy.dataMap.erase(buffer_itr);
        }
        else
        {
            ++buffer_itr;
        }
    }
}

void TransferTask::assign(const ImageInfoList& imageInfoList)
{
    CPU_INSTRUMENTATION_L2(instrumentation);

    std::scoped_lock<std::mutex> lock(_mutex);

    log(level, "TransferTask::assign(ImageInfoList) ", this, ", imageInfoList.size() = ", imageInfoList.size());

    for (auto& imageInfo : imageInfoList)
    {
        if (imageInfo->imageView && imageInfo->imageView && imageInfo->imageView->image->data)
        {
            log(level, "    imageInfo ", imageInfo, ", ", imageInfo->imageView, ", ", imageInfo->imageView->image, ", ", imageInfo->imageView->image->data);
            DataToCopy& dataToCopy = (imageInfo->imageView->image->data->properties.dataVariance >= DYNAMIC_DATA_TRANSFER_AFTER_RECORD) ? _lateDataToCopy : _earlyDataToCopy;
            dataToCopy.imageInfoSet.insert(imageInfo);
        }
    }
}

void TransferTask::_transferImageInfos(DataToCopy& dataToCopy, VkCommandBuffer vk_commandBuffer, TransferBlock& frame, VkDeviceSize& offset)
{
    CPU_INSTRUMENTATION_L1(instrumentation);

    auto deviceID = device->deviceID;

    // transfer any modified ImageInfo
    for (auto imageInfo_itr = dataToCopy.imageInfoSet.begin(); imageInfo_itr != dataToCopy.imageInfoSet.end();)
    {
        auto& imageInfo = *imageInfo_itr;
        if (imageInfo->referenceCount() == 1)
        {
            log(level, "ImageInfo only ref left ", imageInfo, ", ", imageInfo->referenceCount());
            imageInfo_itr = dataToCopy.imageInfoSet.erase(imageInfo_itr);
        }
        else
        {
            if (imageInfo->syncModifiedCounts(deviceID))
            {
                _transferImageInfo(vk_commandBuffer, frame, offset, *imageInfo);
            }
            else
            {
                log(level, "    no need to copy ", imageInfo);
            }

            if (imageInfo->imageView->image->data->dynamic())
            {
                ++imageInfo_itr;
            }
            else
            {
                log(level, "    removing copied static image data: ", imageInfo, ", ", imageInfo->imageView->image->data);
                imageInfo_itr = dataToCopy.imageInfoSet.erase(imageInfo_itr);
            }
        }
    }
}

void TransferTask::_transferImageInfo(VkCommandBuffer vk_commandBuffer, TransferBlock& frame, VkDeviceSize& offset, ImageInfo& imageInfo)
{
    CPU_INSTRUMENTATION_L2(instrumentation);

    auto& data = imageInfo.imageView->image->data;

    VkDeviceSize image_alignment = std::max(static_cast<VkDeviceSize>(data->stride()), static_cast<VkDeviceSize>(4));
    offset = ((offset % image_alignment) == 0) ? offset : ((offset / image_alignment) + 1) * image_alignment;

    auto& imageStagingBuffer = frame.staging;
    auto& buffer_data = frame.buffer_data;
    char* ptr = reinterpret_cast<char*>(buffer_data) + offset;

    auto properties = data->properties;
    auto width = data->width();
    auto height = data->height();
    auto depth = data->depth();

    uint32_t mipLevels = imageInfo.imageView->image->mipLevels;

    auto source_offset = offset;

    log(level, "  TransferTask::_transferImageInfo(..) ", this, ",ImageInfo needs copying ", data, ", mipLevels = ", mipLevels);

    // copy data.
    VkFormat sourceFormat = data->properties.format;
    VkFormat targetFormat = imageInfo.imageView->format;
    if (sourceFormat == targetFormat)
    {
        log(level, "    sourceFormat and targetFormat compatible.");
        std::memcpy(ptr, data->dataPointer(), data->dataSize());
        offset += data->dataSize();
    }
    else
    {
        auto sourceTraits = getFormatTraits(sourceFormat);
        auto targetTraits = getFormatTraits(targetFormat);
        if (sourceTraits.size == targetTraits.size)
        {
            log(level, "    sourceTraits.size and targetTraits.size compatible.");
            std::memcpy(ptr, data->dataPointer(), data->dataSize());
            offset += data->dataSize();
        }
        else
        {
            VkDeviceSize imageTotalSize = targetTraits.size * data->valueCount();

            properties.format = targetFormat;
            properties.stride = targetTraits.size;

            log(level, "    sourceTraits.size and targetTraits.size not compatible. dataSize() = ", data->dataSize(), ", imageTotalSize = ", imageTotalSize);

            // set up a vec4 worth of default values for the type
            const uint8_t* default_ptr = targetTraits.defaultValue;
            uint32_t bytesFromSource = sourceTraits.size;
            uint32_t bytesToTarget = targetTraits.size;

            offset += imageTotalSize;

            // copy data
            using value_type = uint8_t;
            const value_type* src_ptr = reinterpret_cast<const value_type*>(data->dataPointer());

            value_type* dest_ptr = reinterpret_cast<value_type*>(ptr);

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
        }
    }

    // transfer data.
    transferImageData(imageInfo.imageView, imageInfo.imageLayout, properties, width, height, depth, mipLevels, imageStagingBuffer, source_offset, vk_commandBuffer, device);
}

TransferTask::TransferResult TransferTask::transferData(TransferMask transferMask)
{
    log(level, "TransferTask::transferData(", transferMask, ")");

    TransferTask::TransferResult result;
    if ((transferMask & TRANSFER_BEFORE_RECORD_TRAVERSAL) != 0) result = _transferData(_earlyDataToCopy);
    if ((transferMask & TRANSFER_AFTER_RECORD_TRAVERSAL) != 0) result = _transferData(_lateDataToCopy);
    return result;
}

TransferTask::TransferResult TransferTask::_transferData(DataToCopy& dataToCopy)
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "transferData", COLOR_RECORD);

    std::scoped_lock<std::mutex> lock(_mutex);

    log(level, "TransferTask::_transferData( ", dataToCopy.name, " ) ", this, ", frameIndex = ", dataToCopy.frameIndex);

    uint32_t deviceID = device->deviceID;

    // check to see if any copies are require.
    if (!dataToCopy.requiresCopy(deviceID))
    {
        return TransferResult{VK_SUCCESS, {}};
    }

    //
    // begin compute total data size
    //
    VkDeviceSize offset = 0;
    VkDeviceSize alignment = 4;

    for (const auto& imageInfo : dataToCopy.imageInfoSet)
    {
        auto data = imageInfo->imageView->image->data;

        // adjust offset to make sure it fits with the stride();
        VkDeviceSize image_alignment = std::max(static_cast<VkDeviceSize>(data->stride()), alignment);
        offset = ((offset % image_alignment) == 0) ? offset : ((offset / image_alignment) + 1) * image_alignment;

        VkFormat targetFormat = imageInfo->imageView->format;
        auto targetTraits = getFormatTraits(targetFormat);
        VkDeviceSize imageSize = (targetTraits.size > 0) ? targetTraits.size * data->valueCount() : data->dataSize();
        log(level, "      ", data, ", data->dataSize() = ", data->dataSize(), ", imageSize = ", imageSize, " targetTraits.size = ", targetTraits.size, ", ", data->valueCount(), ", targetFormat = ", targetFormat);

        VkDeviceSize endOfEntry = offset + imageSize;
        offset = (/*alignment == 1 ||*/ (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
    }
    dataToCopy.imageTotalSize = offset;

    log(level, "    dataToCopy.imageTotalSize = ", dataToCopy.imageTotalSize);

    offset = 0;
    dataToCopy.dataTotalRegions = 0;
    for (const auto& entry : dataToCopy.dataMap)
    {
        const auto& bufferInfos = entry.second;
        for (const auto& offset_bufferInfo : bufferInfos)
        {
            const auto& bufferInfo = offset_bufferInfo.second;
            VkDeviceSize endOfEntry = offset + bufferInfo->range;
            offset = (/*alignment == 1 ||*/ (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
            ++dataToCopy.dataTotalRegions;
        }
    }
    dataToCopy.dataTotalSize = offset;
    log(level, "    dataToCopy.dataTotalSize = ", dataToCopy.dataTotalSize);

    //
    // end of compute size
    //

    VkDeviceSize totalSize = dataToCopy.dataTotalSize + dataToCopy.imageTotalSize;
    if (totalSize == 0) return TransferResult{VK_SUCCESS, {}};

    log(level, "    totalSize = ", totalSize);

    auto& frame = *(dataToCopy.frames[dataToCopy.frameIndex]);
    auto& fence = frame.fence;
    auto& staging = frame.staging;
    auto& commandBuffer = frame.transferCommandBuffer;
    auto& newSignalSemaphore = dataToCopy.transferCompleteSemaphore;

    const auto& copyRegions = frame.copyRegions;
    auto& buffer_data = frame.buffer_data;

    log(level, "    frameIndex = ", dataToCopy.frameIndex);
    log(level, "    frame = ", &frame);
    log(level, "    fence = ", fence);
    log(level, "    transferQueue = ", transferQueue);
    log(level, "    staging = ", staging);
    log(level, "    dataToCopy.transferConsumerCompletedSemaphore = ", dataToCopy.transferConsumerCompletedSemaphore, ", ", dataToCopy.transferConsumerCompletedSemaphore ? dataToCopy.transferConsumerCompletedSemaphore->vk() : VK_NULL_HANDLE);
    log(level, "    newSignalSemaphore = ", newSignalSemaphore, ", ", newSignalSemaphore ? newSignalSemaphore->vk() : VK_NULL_HANDLE);
    log(level, "    copyRegions.size() = ", copyRegions.size());

    if (frame.waitOnFence && fence)
    {
        uint64_t timeout = std::numeric_limits<uint64_t>::max();
        if (VkResult result = fence->wait(timeout); result != VK_SUCCESS) return TransferResult{result, {}};
        fence->resetFenceAndDependencies();
    }
    frame.waitOnFence = false;

    // advance frameIndex
    dataToCopy.frameIndex = (dataToCopy.frameIndex + 1) % dataToCopy.frames.size();

    if (!commandBuffer)
    {
        auto cp = CommandPool::create(device, transferQueue->queueFamilyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        commandBuffer = cp->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }
    else
    {
        commandBuffer->reset();
    }

    if (!newSignalSemaphore)
    {
        // signal transfer submission has completed
        newSignalSemaphore = Semaphore::create(device, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        log(level, "    newSignalSemaphore created ", newSignalSemaphore, ", ", newSignalSemaphore->vk());
    }

    if (!fence) fence = Fence::create(device);

    VkResult result = VK_SUCCESS;

    // allocate staging buffer if required
    if (!staging || staging->size < totalSize)
    {
        if (totalSize < minimumStagingBufferSize)
        {
            totalSize = minimumStagingBufferSize;
            log(level, "    Clamping totalSize to ", minimumStagingBufferSize);
        }

        VkDeviceSize previousSize = staging ? staging->size : 0;

        VkMemoryPropertyFlags stagingMemoryPropertiesFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        staging = vsg::createBufferAndMemory(device, totalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, stagingMemoryPropertiesFlags);

        auto stagingMemory = staging->getDeviceMemory(deviceID);
        buffer_data = nullptr;
        result = stagingMemory->map(staging->getMemoryOffset(deviceID), staging->size, 0, &buffer_data);

        log(level, "    TransferTask::transferData() frameIndex = ", dataToCopy.frameIndex, ", previousSize = ", previousSize, ", allocated staging buffer = ", staging, ", totalSize = ", totalSize, ", result = ", result);

        if (result != VK_SUCCESS) return TransferResult{VK_SUCCESS, {}};
    }

    log(level, "    totalSize = ", totalSize);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    VkCommandBuffer vk_commandBuffer = *commandBuffer;
    vkBeginCommandBuffer(vk_commandBuffer, &beginInfo);

    offset = 0;
    {
        COMMAND_BUFFER_INSTRUMENTATION(instrumentation, *commandBuffer, "transferData", COLOR_GPU)

        // transfer the modified BufferInfo and ImageInfo
        _transferImageInfos(dataToCopy, vk_commandBuffer, frame, offset);
        _transferBufferInfos(dataToCopy, vk_commandBuffer, frame, offset);
    }

    vkEndCommandBuffer(vk_commandBuffer);

    // if no regions to copy have been found then commandBuffer will be empty so no need to submit it to queue and signal the associated semaphore
    if (offset > 0)
    {
        // submit the transfer commands
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // set up vulkan wait semaphore
        std::vector<VkSemaphore> vk_waitSemaphores;
        std::vector<VkPipelineStageFlags> vk_waitStages;
        if (dataToCopy.transferConsumerCompletedSemaphore)
        {
            vk_waitSemaphores.emplace_back(dataToCopy.transferConsumerCompletedSemaphore->vk());
            vk_waitStages.emplace_back(dataToCopy.transferConsumerCompletedSemaphore->pipelineStageFlags());

            log(level, "TransferTask::_transferData( ", dataToCopy.name, " ) submit dataToCopy.transferConsumerCompletedSemaphore = ", dataToCopy.transferConsumerCompletedSemaphore);
        }

        // set up the vulkan signal semaphore
        std::vector<VkSemaphore> vk_signalSemaphores;

        vk_signalSemaphores.push_back(*newSignalSemaphore);

        submitInfo.waitSemaphoreCount = static_cast<uint32_t>(vk_waitSemaphores.size());
        submitInfo.pWaitSemaphores = vk_waitSemaphores.data();
        submitInfo.pWaitDstStageMask = vk_waitStages.data();
        submitInfo.signalSemaphoreCount = static_cast<uint32_t>(vk_signalSemaphores.size());
        submitInfo.pSignalSemaphores = vk_signalSemaphores.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vk_commandBuffer;

        log(level, "   TransferTask submitInfo.waitSemaphoreCount = ", submitInfo.waitSemaphoreCount);
        log(level, "   TransferTask submitInfo.signalSemaphoreCount = ", submitInfo.signalSemaphoreCount);

        result = transferQueue->submit(submitInfo, fence);

        frame.waitOnFence = true;

        dataToCopy.transferConsumerCompletedSemaphore.reset();

        if (result != VK_SUCCESS) return TransferResult{result, {}};

        return TransferResult{VK_SUCCESS, newSignalSemaphore};
    }
    else
    {
        log(level, "Nothing to submit");
        return TransferResult{VK_SUCCESS, {}};
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::transferImageData(..)
//
void vsg::transferImageData(ref_ptr<ImageView> imageView, VkImageLayout targetImageLayout, Data::Properties properties, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, ref_ptr<Buffer> stagingBuffer, VkDeviceSize stagingBufferOffset, VkCommandBuffer commandBuffer, vsg::Device* device)
{
    auto image = imageView->image;
    if (!image) return;

    auto data = image->data;
    if (!data) return;

    auto vk_image = image->vk(device->deviceID);
    auto aspectMask = imageView->subresourceRange.aspectMask;

    // Stage which the associated image will be used, such as set by DescriptorSetLayoutBinding::stageFlags
    // Further info:
    //     https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineStageFlags.html
    //     https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDescriptorSetLayoutBinding.html
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

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

    uint32_t data_mipLevels = static_cast<uint32_t>(data->properties.mipLevels);

    auto mipmapData = data->getMipmapLayout();
    if (mipmapData)
    {
        const auto& mipmap0 = mipmapData->at(0);
        destWidth = mipmap0.x;
        destHeight = mipmap0.y;
        destDepth = mipmap0.z;

        if (mipmapData->size() > 1)
        {
            data_mipLevels = static_cast<uint32_t>(mipmapData->size());
        }
    }

    bool useDataMipmaps = mipLevels > 1 && data_mipLevels > 1;
    bool generateMipmaps = (mipLevels > 1) && !useDataMipmaps;

    if (useDataMipmaps) mipLevels = std::min(mipLevels, data_mipLevels);

    // vsg::info("vsg::transferImageData() data = ", data, ", data->properties.mipLevels = ", int(data->properties.mipLevels), ", data_mipLevels = ", data_mipLevels, " mipLevels = ", mipLevels, ", mipmapData = ", mipmapData, ", generateMipmaps = ", generateMipmaps);

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
    preCopyBarrier.image = vk_image;
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
        if (mipmapData)
        {
            regions.resize(mipLevels * arrayLayers);

            auto mipmapItr = mipmapData->begin();
            for (uint32_t mipLevel = 0; mipLevel < mipLevels; ++mipLevel)
            {
                const auto& mipmap = (*mipmapItr++);

                for (uint32_t face = 0; face < arrayLayers; ++face)
                {
                    auto& region = regions[mipLevel * arrayLayers + face];
                    region.bufferOffset = stagingBufferOffset + mipmap.w;
                    region.bufferRowLength = 0;
                    region.bufferImageHeight = 0;
                    region.imageSubresource.aspectMask = aspectMask;
                    region.imageSubresource.mipLevel = mipLevel;
                    region.imageSubresource.baseArrayLayer = face;
                    region.imageSubresource.layerCount = 1;
                    region.imageOffset = {0, 0, 0};
                    region.imageExtent = {mipmap.x, mipmap.y, mipmap.z};
                }
            }
        }
        else
        {
            regions.resize(mipLevels * arrayLayers);

            size_t offset = 0u;
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

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer->vk(device->deviceID), vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(regions.size()), regions.data());

    if (generateMipmaps)
    {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = vk_image;
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

            int32_t nextWidth = (mipWidth > 1) ? (mipWidth) / 2 : 1;
            int32_t nextHeight = (mipHeight > 1) ? (mipHeight) / 2 : 1;
            int32_t nextDepth = (mipDepth > 1) ? (mipDepth) / 2 : 1;

            VkImageBlit blit;
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mipWidth, mipHeight, mipDepth};
            blit.srcSubresource.aspectMask = aspectMask;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = arrayLayers;
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {nextWidth, nextHeight, nextDepth};
            blit.dstSubresource.aspectMask = aspectMask;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = arrayLayers;

            vkCmdBlitImage(commandBuffer,
                           vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &blit,
                           VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = targetImageLayout;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, dstStageMask, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            mipWidth = nextWidth;
            mipHeight = nextHeight;
            mipDepth = nextDepth;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = targetImageLayout;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, dstStageMask, 0,
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
        postCopyBarrier.image = vk_image;
        postCopyBarrier.subresourceRange.aspectMask = aspectMask;
        postCopyBarrier.subresourceRange.baseArrayLayer = 0;
        postCopyBarrier.subresourceRange.layerCount = arrayLayers;
        postCopyBarrier.subresourceRange.levelCount = mipLevels;
        postCopyBarrier.subresourceRange.baseMipLevel = 0;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, dstStageMask, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &postCopyBarrier);
    }
}
