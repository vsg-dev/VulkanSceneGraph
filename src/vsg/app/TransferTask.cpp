/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/TransferTask.h>
#include <vsg/app/View.h>
#include <vsg/io/Logger.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/utils/Instrumentation.h>
#include <vsg/vk/State.h>

#include <algorithm>
#include <cassert>
#include <numeric>

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

namespace {
constexpr VkDeviceSize alignOffset(VkDeviceSize offset, VkDeviceSize alignment)
{
    return (/*alignment == 1 ||*/ (offset % alignment) == 0) ? offset : ((offset / alignment) + 1) * alignment;
}
}

void TransferTask::_transferBufferInfos(DataToCopy& dataToCopy, VkCommandBuffer vk_commandBuffer, TransferBlock& frame, size_t stagingBufferOffsetBegin, size_t regionOffsetBegin)
{
    CPU_INSTRUMENTATION_L1(instrumentation);

    auto deviceID = device->deviceID;
    auto& stagingBuffers = frame.stagingBuffers;
    auto& stagingOffsets = frame.stagingOffsets;
    auto& copyRegions = frame.copyRegions;
    auto& buffer_data = frame.buffer_data;

    copyRegions.clear();
    copyRegions.resize(dataToCopy.dataTotalRegions);
    VkBufferCopy* pRegions = copyRegions.data();

    log(level, "  TransferTask::_transferBufferInfos(..) ", this);

    size_t sbOffset = stagingBufferOffsetBegin;

    // copy the buffer regions and advance to next buffer
    const auto copyBufferRegions = [&](ref_ptr<Buffer> buffer, size_t regionCount){
        vkCmdCopyBuffer(vk_commandBuffer, stagingBuffers[sbOffset]->vk(deviceID), buffer->vk(deviceID), regionCount, pRegions);
        log(level, "   vkCmdCopyBuffer(", ", ", stagingBuffers[sbOffset]->vk(deviceID), ", ", buffer->vk(deviceID), ", ", regionCount, ", ", pRegions);
        return pRegions + regionCount;
    };

    // copy any modified BufferInfo
    size_t regionOffset = regionOffsetBegin;
    for (auto buffer_itr = dataToCopy.dataMap.begin(); buffer_itr != dataToCopy.dataMap.end();)
    {
        auto& bufferInfos = buffer_itr->second;

        size_t regionCount = 0;
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
                    assert(sbOffset < stagingOffsets.size() && regionOffset < stagingOffsets[sbOffset].size()-1);
                    VkDeviceSize offset = stagingOffsets[sbOffset][regionOffset];

                    // copy data to staging buffer memory
                    char* ptr = reinterpret_cast<char*>(buffer_data[sbOffset]) + offset;
                    std::memcpy(ptr, bufferInfo->data->dataPointer(), bufferInfo->range);

                    // record region
                    pRegions[regionCount++] = VkBufferCopy{offset, bufferInfo->offset, bufferInfo->range};

                    log(level, "       copying ", bufferInfo, ", ", bufferInfo->data, " to ", static_cast<void*>(ptr));

                    // advanced to the next staging buffer if it has reached capacity
                    ++regionOffset;
                    if (regionOffset == stagingOffsets[sbOffset].size()-1)
                    {
                        // copy the buffer regions and advance to next buffer
                        pRegions = copyBufferRegions(buffer_itr->first, regionCount);

                        ++sbOffset;
                        regionOffset = 0;
                        regionCount = 0;
                    }
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
                    if (bufferInfo->data->properties.dataVariance == STATIC_DATA_UNREF_AFTER_TRANSFER)
                    {
                        bufferInfo->data.reset();
                    }
                    bufferInfo_itr = bufferInfos.erase(bufferInfo_itr);
                }
            }
        }
        log(level, "    } bufferInfos.size() = ", bufferInfos.size(), "{");

        if (regionCount > 0)
        {
            // copy the buffer regions and advance to next buffer
            pRegions = copyBufferRegions(buffer_itr->first, regionCount);
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

void TransferTask::_transferImageInfos(DataToCopy& dataToCopy, VkCommandBuffer vk_commandBuffer, TransferBlock& frame, size_t stagingBufferOffsetBegin, size_t regionOffsetBegin)
{
    CPU_INSTRUMENTATION_L1(instrumentation);

    auto deviceID = device->deviceID;
    auto& stagingOffsets = frame.stagingOffsets;

    // transfer any modified ImageInfo
    size_t sbOffset = stagingBufferOffsetBegin;
    size_t regionOffset = regionOffsetBegin;
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
                assert(sbOffset < stagingOffsets.size() && regionOffset < stagingOffsets[sbOffset].size()-1);
                _transferImageInfo(vk_commandBuffer, frame, sbOffset, regionOffset, *imageInfo);

                // advanced to the next staging buffer if it has reached capacity
                ++regionOffset;
                if (regionOffset == stagingOffsets[sbOffset].size()-1)
                {
                    ++sbOffset;
                    regionOffset = 0;
                }
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

void TransferTask::_transferImageInfo(VkCommandBuffer vk_commandBuffer, TransferBlock& frame, size_t stagingBufferOffset, size_t regionOffset, ImageInfo& imageInfo)
{
    CPU_INSTRUMENTATION_L2(instrumentation);

    auto& data = imageInfo.imageView->image->data;

    auto& imageStagingBuffer = frame.stagingBuffers[stagingBufferOffset];
    const auto offset = frame.stagingOffsets[stagingBufferOffset][regionOffset];
    auto& buffer_data = frame.buffer_data[stagingBufferOffset];
    char* ptr = reinterpret_cast<char*>(buffer_data) + offset;

    auto properties = data->properties;
    auto width = data->width();
    auto height = data->height();
    auto depth = data->depth();
    auto mipmapOffsets = data->computeMipmapOffsets();
    uint32_t mipLevels = vsg::computeNumMipMapLevels(data, imageInfo.sampler);

    log(level, "  TransferTask::_transferImageInfo(..) ", this, ",ImageInfo needs copying ", data, ", mipLevels = ", mipLevels);

    // copy data.
    VkFormat sourceFormat = data->properties.format;
    VkFormat targetFormat = imageInfo.imageView->format;
    if (sourceFormat == targetFormat)
    {
        log(level, "    sourceFormat and targetFormat compatible.");
        std::memcpy(ptr, data->dataPointer(), data->dataSize());
    }
    else
    {
        auto sourceTraits = getFormatTraits(sourceFormat);
        auto targetTraits = getFormatTraits(targetFormat);
        if (sourceTraits.size == targetTraits.size)
        {
            log(level, "    sourceTraits.size and targetTraits.size compatible.");
            std::memcpy(ptr, data->dataPointer(), data->dataSize());
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
    transferImageData(imageInfo.imageView, imageInfo.imageLayout, properties, width, height, depth, mipLevels, mipmapOffsets, imageStagingBuffer, offset, vk_commandBuffer, device);
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
    // begin gathering staging buffer offsets
    //
    const VkDeviceSize alignment = 4;

    const auto maxMemoryAllocationSize = device->getPhysicalDevice()
        ->getProperties<VkPhysicalDeviceMaintenance3Properties,
              VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES>().maxMemoryAllocationSize;

    const auto frameIndex = dataToCopy.frameIndex;
    auto& frame = *(dataToCopy.frames[frameIndex]);
    auto& stagingOffsets = frame.stagingOffsets;

    // Allocate a vector of offsets for the new staging buffer with the first offset initialized to
    // zero for the new entry, returning the end offset of the new entry.
    const auto addRegionToNewStagingBuffer = [&](VkDeviceSize regionSize){
        stagingOffsets.push_back(std::vector<VkDeviceSize>(1,VkDeviceSize(0)));
        return alignOffset(regionSize, alignment);
    };

    //
    // Gather image offsets into one or more staging buffers of the frame. If multiple images do not
    // fit within a single staging buffer, additional staging buffers are created to accommodate the
    // transfer. Any single ImageInfo data must fit within the maximum allocation size for a single
    // staging buffer, however some drivers do permit overallocation so if there is a single allocation
    // larger that the maximum we rely on the memory allocation request throwing an exception if it
    // is unable to honor the request. Ideally the client code should create smaller buffers.
    //
    VkDeviceSize offset = 0;
    stagingOffsets.clear();
    const size_t imageStagingBufferOffsetBegin = 0;
    const size_t imageRegionOffsetBegin = 0;
    if (!dataToCopy.imageInfoSet.empty())
    {
        for (const auto& imageInfo : dataToCopy.imageInfoSet)
        {
            // see TransferTask::_transferImageInfos
            if (imageInfo->referenceCount() > 1 && imageInfo->requiresCopy(deviceID))
            {
                // if this is the first staging buffer, allocate a vector of offsets
                if (stagingOffsets.empty())
                    stagingOffsets.push_back({});

                auto data = imageInfo->imageView->image->data;

                // adjust offset to make sure it fits with the stride();
                const VkDeviceSize image_alignment = std::max(static_cast<VkDeviceSize>(data->stride()), alignment);
                offset = alignOffset(offset, image_alignment);
                stagingOffsets.back().push_back(offset);

                VkFormat targetFormat = imageInfo->imageView->format;
                auto targetTraits = getFormatTraits(targetFormat);
                VkDeviceSize imageSize = (targetTraits.size > 0) ? targetTraits.size * data->valueCount() : data->dataSize();
                log(level, "      ", data, ", data->dataSize() = ", data->dataSize(), ", imageSize = ", imageSize, " targetTraits.size = ", targetTraits.size, ", ", data->valueCount(), ", targetFormat = ", targetFormat);

                offset = alignOffset(offset + imageSize, alignment);

                // If adding this region would exceed the maximum allocation size create a new
                // staging buffer, unless this is an overallocation request and it is the only
                // allocation in the buffer, in which case permit the overallocation to reside
                // in this staging buffer.
                if (offset >= maxMemoryAllocationSize && stagingOffsets.back().size() > 1)
                    offset = addRegionToNewStagingBuffer(imageSize);
            }
        }
    }

    const size_t numImageStagingBuffers = stagingOffsets.size();
    const size_t numImageRegionsInLastBuffer = stagingOffsets.empty() ? 0 : stagingOffsets.back().size();

    //
    // Gather data offsets into one or more staging buffers of the frame. If multiple data buffers
    // do not fit within a single staging buffer, additional staging buffers are created to
    // accommodate the transfer. Any single data buffer must fit within the maximum allocation size
    // for a single staging buffer, however some drivers do permit overallocation so if there is a
    // single allocation larger that the maximum we rely on the memory allocation request throwing
    // an exception if it is unable to honor the request. Ideally the client code should create
    // smaller buffers.
    //
    dataToCopy.dataTotalRegions = 0;
    if (!dataToCopy.dataMap.empty())
    {
        for (const auto&[_,bufferInfos] : dataToCopy.dataMap)
        {
            for (const auto& offset_bufferInfo : bufferInfos)
            {
                const auto& bufferInfo = offset_bufferInfo.second;

                // see TransferTask::_transferBufferInfos
                if (bufferInfo->referenceCount() > 1 && bufferInfo->requiresCopy(deviceID))
                {
                    // if this is the first staging buffer, allocate a vector of offsets
                    if (stagingOffsets.empty())
                        stagingOffsets.push_back({});

                    stagingOffsets.back().push_back(offset);

                    ++dataToCopy.dataTotalRegions;

                    offset = alignOffset(offset + bufferInfo->range, alignment);

                    // If adding this region would exceed the maximum allocation size create a new
                    // staging buffer, unless this is an overallocation request and it is the only
                    // allocation in the buffer, in which case permit the overallocation to reside
                    // in this staging buffer.
                    if (offset >= maxMemoryAllocationSize && stagingOffsets.back().size() > 1)
                        offset = addRegionToNewStagingBuffer(bufferInfo->range);
                }
            }
        }
    }

    // If any staging buffer offsets were added, record the exclusive ending offset of the previous
    // entry for the last staging buffer. The last offset is used to get each staging buffer's size.
    if (!stagingOffsets.empty())
    {
        // If adding this region would exceed the maximum allocation size create a new
        // staging buffer, unless this is an overallocation request and it is the only
        // allocation in the buffer, in which case permit the overallocation to reside
        // in this staging buffer.
        if (offset >= maxMemoryAllocationSize && stagingOffsets.back().size() > 1)
            offset = addRegionToNewStagingBuffer(offset - stagingOffsets.back().back());

        stagingOffsets.back().push_back(offset);
    }

    // compute the starting staging buffer and offsets for the data regions
    size_t dataStagingBufferOffsetBegin = 0;
    size_t dataRegionOffsetBegin = 0;
    if (numImageStagingBuffers > 0)
    {
        if (numImageRegionsInLastBuffer < stagingOffsets[numImageStagingBuffers-1].size()-1)
        {
            // the beginning of the data regions share the same staging buffer as the end of the images
            dataStagingBufferOffsetBegin = numImageStagingBuffers-1;
            dataRegionOffsetBegin = numImageRegionsInLastBuffer;
        }
        else
        {
            // data regions were added to a new staging buffers
            dataStagingBufferOffsetBegin = numImageStagingBuffers;
            dataRegionOffsetBegin = 0;
        }
    }

    log(level, "    dataToCopy total size = ",
        std::accumulate(stagingOffsets.begin(), stagingOffsets.end(), VkDeviceSize(0),
            [](auto sum, const auto& offsets){ return sum + offsets.back(); }));

    //
    // end of gathering staging buffer offsets
    //

    auto& fence = frame.fence;
    auto& stagingBuffers = frame.stagingBuffers;
    auto& commandBuffer = frame.transferCommandBuffer;
    auto& newSignalSemaphore = dataToCopy.transferCompleteSemaphore;
    auto& buffer_data = frame.buffer_data;

    log(level, "    frameIndex = ", frameIndex);
    log(level, "    frame = ", &frame);
    log(level, "    fence = ", fence);
    log(level, "    transferQueue = ", transferQueue);
    log(level, "    dataToCopy.transferConsumerCompletedSemaphore = ", dataToCopy.transferConsumerCompletedSemaphore, ", ", dataToCopy.transferConsumerCompletedSemaphore ? dataToCopy.transferConsumerCompletedSemaphore->vk() : VK_NULL_HANDLE);
    log(level, "    newSignalSemaphore = ", newSignalSemaphore, ", ", newSignalSemaphore ? newSignalSemaphore->vk() : VK_NULL_HANDLE);

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
        auto cp = CommandPool::create(device, transferQueue->queueFamilyIndex());
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

    // if no regions to copy have been found then commandBuffer will be empty so no need to submit it to queue and signal the associated semaphore
    if (!stagingOffsets.empty())
    {
        // allocate/reallocate staging buffers as required
        stagingBuffers.resize(stagingOffsets.size());
        buffer_data.resize(stagingOffsets.size());

        for (size_t sbOffset = 0; sbOffset < stagingBuffers.size(); ++sbOffset)
        {
            // The last entry in each staging offsets vector is the aligned ending offset of the
            // last entry which is the requested staging buffer size. Compute the staging buffer
            // size so that it is always a multiple of the allocation chunk size and no smaller than
            // the minimum staging buffer size. For the special case of an overallocation request
            // for a single region just pass the request on and let the driver/OS decide if it can
            // fulfill the request.
            const auto requestedSize = stagingOffsets[sbOffset].back();
            // if this is an overallocation request it should be in its own staging buffer to minimize the request size
            assert(requestedSize <= maxMemoryAllocationSize || stagingOffsets[sbOffset].size() == 2);
            const auto stagingSize = requestedSize > maxMemoryAllocationSize
                ? requestedSize // overallocation request
                : std::clamp(stagingBufferSizeChunkSize * ((requestedSize - 1) / stagingBufferSizeChunkSize + 1),
                             minimumStagingBufferSize, maxMemoryAllocationSize);

            if (!stagingBuffers[sbOffset] || stagingBuffers[sbOffset]->size != stagingSize)
            {
                VkDeviceSize previousSize = stagingBuffers[sbOffset] ? stagingBuffers[sbOffset]->size : 0;

                VkMemoryPropertyFlags stagingMemoryPropertiesFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                stagingBuffers[sbOffset] = vsg::createBufferAndMemory(device, stagingSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, stagingMemoryPropertiesFlags);

                auto stagingMemory = stagingBuffers[sbOffset]->getDeviceMemory(deviceID);
                buffer_data[sbOffset] = nullptr;
                result = stagingMemory->map(stagingBuffers[sbOffset]->getMemoryOffset(deviceID), stagingBuffers[sbOffset]->size, 0, &buffer_data[sbOffset]);

                log(level, "    TransferTask::transferData() frameIndex = ", frameIndex, ", staging buffer offset = ", sbOffset, "previousSize = ", previousSize, ", allocated staging buffer = ", stagingBuffers[sbOffset], ", stagingSize = ", stagingSize, ", result = ", result);

                if (result != VK_SUCCESS) return TransferResult{VK_SUCCESS, {}};
            }
        }

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        VkCommandBuffer vk_commandBuffer = *commandBuffer;
        vkBeginCommandBuffer(vk_commandBuffer, &beginInfo);

        {
            COMMAND_BUFFER_INSTRUMENTATION(instrumentation, *commandBuffer, "transferData", COLOR_GPU)

            // transfer the modified BufferInfo and ImageInfo
            _transferImageInfos(dataToCopy, vk_commandBuffer, frame, imageStagingBufferOffsetBegin, imageRegionOffsetBegin);
            _transferBufferInfos(dataToCopy, vk_commandBuffer, frame, dataStagingBufferOffsetBegin, dataRegionOffsetBegin);
        }

        vkEndCommandBuffer(vk_commandBuffer);

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
