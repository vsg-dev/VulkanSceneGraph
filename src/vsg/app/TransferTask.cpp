/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/TransferTask.h>
#include <vsg/app/View.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/utils/Instrumentation.h>
#include <vsg/vk/State.h>

using namespace vsg;

TransferTask::TransferTask(Device* in_device, uint32_t numBuffers) :
    device(in_device)
{
    CPU_INSTRUMENTATION_L1(instrumentation);

    _currentFrameIndex = numBuffers; // numBuffers is used to signify unset value
    for (uint32_t i = 0; i < numBuffers; ++i)
    {
        _indices.emplace_back(numBuffers); // numBuffers is used to signify unset value
    }

    _frames.resize(numBuffers);

    // level = Logger::LOGGER_INFO;
}

void TransferTask::advance()
{
    CPU_INSTRUMENTATION_L1(instrumentation);
    std::scoped_lock<std::mutex> lock(_mutex);

    if (_currentFrameIndex >= _indices.size())
    {
        // first frame so set to 0
        _currentFrameIndex = 0;
    }
    else
    {
        ++_currentFrameIndex;
        if (_currentFrameIndex > _indices.size() - 1) _currentFrameIndex = 0;

        // shift the index for previous frames
        for (size_t i = _indices.size() - 1; i >= 1; --i)
        {
            _indices[i] = _indices[i - 1];
        }
    }

    // pass the index for the current frame
    _indices[0] = _currentFrameIndex;
}

size_t TransferTask::index(size_t relativeFrameIndex) const
{
    return relativeFrameIndex < _indices.size() ? _indices[relativeFrameIndex] : _indices.size();
}

bool TransferTask::containsDataToTransfer() const
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return !_dynamicDataMap.empty() || !_dynamicImageInfoSet.empty();
}

void TransferTask::assign(const ResourceRequirements::DynamicData& dynamicData)
{
    CPU_INSTRUMENTATION_L1(instrumentation);

    assign(dynamicData.bufferInfos);
    assign(dynamicData.imageInfos);
}

void TransferTask::assign(const BufferInfoList& bufferInfoList)
{
    CPU_INSTRUMENTATION_L1(instrumentation);

    std::scoped_lock<std::mutex> lock(_mutex);

    log(level, "TransferTask::assign(BufferInfoList) ", this, ", ", bufferInfoList.size());

    for (auto& bufferInfo : bufferInfoList)
    {
        log(level, "    bufferInfo ", bufferInfo, " { ", bufferInfo->data, ", ", bufferInfo->buffer, "}");
        _dynamicDataMap[bufferInfo->buffer][bufferInfo->offset] = bufferInfo;
    }

    // compute total data size
    VkDeviceSize offset = 0;
    VkDeviceSize alignment = 4;

    _dynamicDataTotalRegions = 0;
    for (auto& entry : _dynamicDataMap)
    {
        auto& bufferInfos = entry.second;
        for (auto& offset_bufferInfo : bufferInfos)
        {
            auto& bufferInfo = offset_bufferInfo.second;
            VkDeviceSize endOfEntry = offset + bufferInfo->range;
            offset = (/*alignment == 1 ||*/ (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
            ++_dynamicDataTotalRegions;
        }
    }
    _dynamicDataTotalSize = offset;
}

void TransferTask::_transferBufferInfos(VkCommandBuffer vk_commandBuffer, Frame& frame, VkDeviceSize& offset)
{
    CPU_INSTRUMENTATION_L1(instrumentation);

    auto deviceID = device->deviceID;
    auto& staging = frame.staging;
    auto& copyRegions = frame.copyRegions;
    auto& buffer_data = frame.buffer_data;

    VkDeviceSize alignment = 4;

    copyRegions.clear();
    copyRegions.resize(_dynamicDataTotalRegions);
    VkBufferCopy* pRegions = copyRegions.data();

    log(level, "  TransferTask::_transferBufferInfos(..) ", this);

    // copy any modified BufferInfo
    for (auto buffer_itr = _dynamicDataMap.begin(); buffer_itr != _dynamicDataMap.end();)
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

                    log(level, "       copying ", bufferInfo, ", ", bufferInfo->data, " to ", (void*)ptr);

                    VkDeviceSize endOfEntry = offset + bufferInfo->range;
                    offset = (/*alignment == 1 ||*/ (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
                }
                else
                {
                    log(level, "       no need to copy ", bufferInfo);
                }

                if (bufferInfo->data->properties.dataVariance == STATIC_DATA)
                {
                    log(level, "       removing copied static data: ", bufferInfo, ", ", bufferInfo->data);
                    bufferInfo_itr = bufferInfos.erase(bufferInfo_itr);
                }
                else
                {
                    ++bufferInfo_itr;
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
            buffer_itr = _dynamicDataMap.erase(buffer_itr);
        }
        else
        {
            ++buffer_itr;
        }
    }
}

void TransferTask::assign(const ImageInfoList& imageInfoList)
{
    CPU_INSTRUMENTATION_L1(instrumentation);

    std::scoped_lock<std::mutex> lock(_mutex);

    log(level, "TransferTask::assign(imageInfoList) ", this, ", ", imageInfoList.size());
    for (auto& imageInfo : imageInfoList)
    {
        log(level, "    imageInfo ", imageInfo, ", ", imageInfo->imageView, ", ", imageInfo->imageView->image, ", ", imageInfo->imageView->image->data);
        _dynamicImageInfoSet.insert(imageInfo);
    }

    // compute total data size
    VkDeviceSize offset = 0;
    VkDeviceSize alignment = 4;

    for (auto& imageInfo : _dynamicImageInfoSet)
    {
        auto data = imageInfo->imageView->image->data;

        VkFormat targetFormat = imageInfo->imageView->format;
        auto targetTraits = getFormatTraits(targetFormat);
        VkDeviceSize imageTotalSize = targetTraits.size * data->valueCount();

        log(level, "      ", data, ", data->dataSize() = ", data->dataSize(), ", imageTotalSize = ", imageTotalSize);

        VkDeviceSize endOfEntry = offset + imageTotalSize;
        offset = (/*alignment == 1 ||*/ (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
    }
    _dynamicImageTotalSize = offset;

    log(level, "    _dynamicImageTotalSize = ", _dynamicImageTotalSize);
}

void TransferTask::_transferImageInfos(VkCommandBuffer vk_commandBuffer, Frame& frame, VkDeviceSize& offset)
{
    CPU_INSTRUMENTATION_L1(instrumentation);

    auto deviceID = device->deviceID;

    // transfer any modified ImageInfo
    for (auto imageInfo_itr = _dynamicImageInfoSet.begin(); imageInfo_itr != _dynamicImageInfoSet.end();)
    {
        auto& imageInfo = *imageInfo_itr;
        if (imageInfo->referenceCount() == 1)
        {
            log(level, "ImageInfo only ref left ", imageInfo, ", ", imageInfo->referenceCount());
            imageInfo_itr = _dynamicImageInfoSet.erase(imageInfo_itr);
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

            if (imageInfo->imageView->image->data->properties.dataVariance == STATIC_DATA)
            {
                log(level, "       removing copied static image data: ", imageInfo, ", ", imageInfo->imageView->image->data);
                imageInfo_itr = _dynamicImageInfoSet.erase(imageInfo_itr);
            }
            else
            {
                ++imageInfo_itr;
            }
        }
    }
}

void TransferTask::_transferImageInfo(VkCommandBuffer vk_commandBuffer, Frame& frame, VkDeviceSize& offset, ImageInfo& imageInfo)
{
    CPU_INSTRUMENTATION_L1(instrumentation);

    auto& imageStagingBuffer = frame.staging;
    auto& buffer_data = frame.buffer_data;
    char* ptr = reinterpret_cast<char*>(buffer_data) + offset;

    auto& data = imageInfo.imageView->image->data;
    auto properties = data->properties;
    auto width = data->width();
    auto height = data->height();
    auto depth = data->depth();
    auto mipmapOffsets = data->computeMipmapOffsets();
    uint32_t mipLevels = vsg::computeNumMipMapLevels(data, imageInfo.sampler);

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
    transferImageData(imageInfo.imageView, imageInfo.imageLayout, properties, width, height, depth, mipLevels, mipmapOffsets, imageStagingBuffer, source_offset, vk_commandBuffer, device);
}

VkResult TransferTask::transferDynamicData()
{
    CPU_INSTRUMENTATION_L1_NC(instrumentation, "transferDynamicData", COLOR_RECORD);

    std::scoped_lock<std::mutex> lock(_mutex);

    size_t frameIndex = index(0);
    if (frameIndex > _frames.size()) return VK_SUCCESS;

    VkDeviceSize totalSize = _dynamicDataTotalSize + _dynamicImageTotalSize;
    if (totalSize == 0) return VK_SUCCESS;

    uint32_t deviceID = device->deviceID;
    auto& frame = _frames[frameIndex];
    auto& staging = frame.staging;
    auto& commandBuffer = frame.transferCommandBuffer;
    auto& semaphore = frame.transferCompleteSemaphore;
    const auto& copyRegions = frame.copyRegions;
    auto& buffer_data = frame.buffer_data;

    log(level, "\nTransferTask::transferDynamicData() ", this, ", _currentFrameIndex = ", _currentFrameIndex, ", _dynamicDataMap.size() ", _dynamicDataMap.size());
    log(level, "   frameIndex = ", frameIndex);
    log(level, "   transferQueue = ", transferQueue);
    log(level, "   staging = ", staging);
    log(level, "   semaphore = ", semaphore, ", ", semaphore ? semaphore->vk() : VK_NULL_HANDLE);
    log(level, "   copyRegions.size() = ", copyRegions.size());

    if (!commandBuffer)
    {
        auto cp = CommandPool::create(device, transferQueue->queueFamilyIndex());
        commandBuffer = cp->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }
    else
    {
        commandBuffer->reset();
    }

    if (!semaphore)
    {
        // signal transfer submission has completed
        semaphore = Semaphore::create(device, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        log(level, "   Semaphore created ", semaphore, ", ", semaphore->vk());
    }

    VkResult result = VK_SUCCESS;

    // allocate staging buffer if required
    if (!staging || staging->size < totalSize)
    {
        VkMemoryPropertyFlags stagingMemoryPropertiesFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        staging = vsg::createBufferAndMemory(device, totalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, stagingMemoryPropertiesFlags);

        auto stagingMemory = staging->getDeviceMemory(deviceID);
        buffer_data = nullptr;
        result = stagingMemory->map(staging->getMemoryOffset(deviceID), staging->size, 0, &buffer_data);

        log(level, "   allocated staging buffer = ", staging, ", totalSize = ", totalSize, ", result = ", result);


        if (result != VK_SUCCESS) return result;
    }

    log(level, "   totalSize = ", totalSize);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    VkCommandBuffer vk_commandBuffer = *commandBuffer;
    vkBeginCommandBuffer(vk_commandBuffer, &beginInfo);

    VkDeviceSize offset = 0;
    {
        COMMAND_BUFFER_INSTRUMENTATION(instrumentation, *commandBuffer, "transferDynamicData", COLOR_GPU)

        // transfer the modified BufferInfo and ImageInfo
        _transferBufferInfos(vk_commandBuffer, frame, offset);
        _transferImageInfos(vk_commandBuffer, frame, offset);
    }

    vkEndCommandBuffer(vk_commandBuffer);

    // if no regions to copy have been found then commandBuffer will be empty so no need to submit it to queue and signal the associated semaphore
    if (offset > 0)
    {
        // submit the transfer commands
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // set up the vulkan wait sempahore
        std::vector<VkSemaphore> vk_waitSemaphores;
        std::vector<VkPipelineStageFlags> vk_waitStages;
        if (waitSemaphores.empty())
        {
            submitInfo.waitSemaphoreCount = 0;
            submitInfo.pWaitSemaphores = nullptr;
            submitInfo.pWaitDstStageMask = nullptr;
            // info("TransferTask::transferDynamicData() ", this, ", _currentFrameIndex = ", _currentFrameIndex);
        }
        else
        {
            for (auto& waitSemaphore : waitSemaphores)
            {
                vk_waitSemaphores.emplace_back(*(waitSemaphore));
                vk_waitStages.emplace_back(waitSemaphore->pipelineStageFlags());
            }

            submitInfo.waitSemaphoreCount = static_cast<uint32_t>(vk_waitSemaphores.size());
            submitInfo.pWaitSemaphores = vk_waitSemaphores.data();
            submitInfo.pWaitDstStageMask = vk_waitStages.data();
        }

        // set up the vulkan signal sempahore
        std::vector<VkSemaphore> vk_signalSemaphores;
        vk_signalSemaphores.push_back(*semaphore);
        for (auto& ss : signalSemaphores)
        {
            vk_signalSemaphores.push_back(*ss);
        }

        submitInfo.signalSemaphoreCount = static_cast<uint32_t>(vk_signalSemaphores.size());
        submitInfo.pSignalSemaphores = vk_signalSemaphores.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vk_commandBuffer;

        result = transferQueue->submit(submitInfo);

        waitSemaphores.clear();

        if (result != VK_SUCCESS) return result;

        currentTransferCompletedSemaphore = semaphore;
    }
    else
    {
        log(level, "Nothing to submit");

        waitSemaphores.clear();
    }

    return VK_SUCCESS;
}
