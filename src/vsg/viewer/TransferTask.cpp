/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/traversals/RecordTraversal.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/viewer/TransferTask.h>
#include <vsg/viewer/View.h>
#include <vsg/vk/State.h>

using namespace vsg;

TransferTask::TransferTask(Device* in_device, uint32_t numBuffers) :
    device(in_device)
{
    _currentFrameIndex = numBuffers; // numBuffers is used to signify unset value
    for (uint32_t i = 0; i < numBuffers; ++i)
    {
        _indices.emplace_back(numBuffers); // numBuffers is used to signify unset value
    }

    _frames.resize(numBuffers);
    for (uint32_t i = 0; i < numBuffers; ++i)
    {
        _frames[i].fence = vsg::Fence::create(device);
    }
}

void TransferTask::advance()
{
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

/// fence() and fence(0) return the Fence for the frame currently being rendered, fence(1) return the previous frame's Fence etc.
Fence* TransferTask::fence(size_t relativeFrameIndex)
{
    size_t i = index(relativeFrameIndex);
    return i < _frames.size() ? _frames[i].fence.get() : nullptr;
}

void TransferTask::assign(const BufferInfoList& bufferInfoList)
{
    for (auto& bufferInfo : bufferInfoList)
    {
        _dynamicDataMap[bufferInfo->buffer][bufferInfo->offset] = bufferInfo;
    }

    // compute total data size
    VkDeviceSize offset = 0;
    VkDeviceSize alignment = 4;

    _dynamicDataTotalRegions = 0;
    for(auto& [buffer, bufferInfos] : _dynamicDataMap)
    {
        for(auto& offset_bufferInfo : bufferInfos)
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
    Logger::Level level = Logger::LOGGER_DEBUG;
    level = Logger::LOGGER_INFO;

    uint32_t deviceID = device->deviceID;
    auto& staging = frame.staging;
    auto& copyRegions = frame.copyRegions;
    auto& buffer_data = frame.buffer_data;

    VkDeviceSize alignment = 4;

    copyRegions.clear();
    copyRegions.resize(_dynamicDataTotalRegions);
    VkBufferCopy* pRegions = copyRegions.data();

    // copy any modified BufferInfo
    for(auto buffer_itr = _dynamicDataMap.begin(); buffer_itr != _dynamicDataMap.end();)
    {
        auto& buffer = buffer_itr->first;
        auto& bufferInfos = buffer_itr->second;

        uint32_t regionCount = 0;
        for(auto bufferInfo_itr = bufferInfos.begin(); bufferInfo_itr != bufferInfos.end();)
        {
            auto& bufferInfo = bufferInfo_itr->second;
            if (bufferInfo->referenceCount()==1)
            {
                log(level, "BufferInfo only ref left ", bufferInfo, ", ", bufferInfo->referenceCount());
                bufferInfo_itr = bufferInfos.erase(bufferInfo_itr);
            }
            else
            {
                if (bufferInfo->data->getModifiedCount(bufferInfo->copiedModifiedCounts[deviceID]))
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
                ++bufferInfo_itr;
            }
        }

        if (regionCount > 0)
        {
            vkCmdCopyBuffer(vk_commandBuffer, staging->vk(deviceID), buffer->vk(deviceID), regionCount, pRegions);

            log(level, "   vkCmdCopyBuffer(", ", ", staging->vk(deviceID), ", ", buffer->vk(deviceID), ", ", regionCount, ", ", pRegions);

            // advance to next buffer
            pRegions += regionCount;
        }

        if (bufferInfos.empty())
        {
            log(level, "bufferInfos.empty()");
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
    info("TransferTask::assign(imageInfoList) ", imageInfoList.size());
    for (auto& imageInfo : imageInfoList)
    {
        info("    imageInfo ", imageInfo, ", ", imageInfo->imageView, ", ", imageInfo->imageView->image, ", ", imageInfo->imageView->image->data);
        _dynamicImageInfoSet.insert(imageInfo);
    }

    // compute total data size
    VkDeviceSize offset = 0;
    VkDeviceSize alignment = 4;

    for(auto& imageInfo : _dynamicImageInfoSet)
    {
        auto data = imageInfo->imageView->image->data;
        VkDeviceSize imageSize = data->dataSize();

        VkDeviceSize endOfEntry = offset + imageSize;
        offset = (/*alignment == 1 ||*/ (endOfEntry % alignment) == 0) ? endOfEntry : ((endOfEntry / alignment) + 1) * alignment;
    }
    _dynamicImageTotalSize = offset;

    info("    _dynamicImageTotalSize = ", _dynamicImageTotalSize);
}

void TransferTask::_transferImageInfos(VkCommandBuffer vk_commandBuffer, Frame& frame, VkDeviceSize& offset)
{
    Logger::Level level = Logger::LOGGER_DEBUG;
    level = Logger::LOGGER_INFO;

    uint32_t deviceID = device->deviceID;
    auto& staging = frame.staging;
    auto& buffer_data = frame.buffer_data;

    // copy any modified ImageInfo
    for(auto imageInfo_itr = _dynamicImageInfoSet.begin(); imageInfo_itr != _dynamicImageInfoSet.end();)
    {
        auto& imageInfo = *imageInfo_itr;
        if (imageInfo->referenceCount() == 1)
        {
            log(level, "ImageInfo only ref left ", imageInfo, ", ", imageInfo->referenceCount());
            imageInfo_itr = _dynamicImageInfoSet.erase(imageInfo_itr);
        }
        else
        {
            auto& data = imageInfo->imageView->image->data;
            char* ptr = reinterpret_cast<char*>(buffer_data) + offset;

            log(level, "ImageInfo needs copying ", data);
#if 0
            if (data->getModifiedCount(imageInfo->copiedModifiedCounts[deviceID]))
            {
                ++numTransferred;
            }
#endif

            VkFormat sourceFormat = data->getLayout().format;
            VkFormat targetFormat = imageInfo->imageView->format;
            if (sourceFormat == targetFormat)
            {
                info("    sourceFormat and targetFormat compatible.");
                std::memcpy(ptr, data->dataPointer(), data->dataSize());
                offset += data->dataSize();
            }
            else
            {
                auto sourceTraits = getFormatTraits(sourceFormat);
                auto targetTraits = getFormatTraits(targetFormat);
                if (sourceTraits.size == targetTraits.size)
                {
                    info("    sourceTraits.size and targetTraits.size compatible.");
                    std::memcpy(ptr, data->dataPointer(), data->dataSize());
                    offset += data->dataSize();
                }
                else
                {
                    VkDeviceSize imageTotalSize = targetTraits.size * data->valueCount();

                    info("    sourceTraits.size and targetTraits.size not compatible. dataSize() = ", data->dataSize(), ", imageTotalSize = ", imageTotalSize);
                }
            }

            ++imageInfo_itr;
        }
    }
}

VkResult TransferTask::transferDynamicData()
{
    Logger::Level level = Logger::LOGGER_DEBUG;
    level = Logger::LOGGER_INFO;

    size_t frameIndex = index(0);
    if (frameIndex > _frames.size()) return VK_SUCCESS;
    if (_dynamicDataMap.empty() && _dynamicImageInfoSet.empty()) return VK_SUCCESS;

    uint32_t deviceID = device->deviceID;
    auto& frame = _frames[frameIndex];
    auto& staging = frame.staging;
    auto& commandBuffer = frame.transferCommandBuffer;
    auto& semaphore = frame.transferCompledSemaphore;
    auto& copyRegions = frame.copyRegions;
    auto& buffer_data = frame.buffer_data;

    log(level, "TransferTask::record() ", _currentFrameIndex, ", _dynamicDataMap.size() ", _dynamicDataMap.size());
    log(level, "   transferQueue = ", transferQueue);
    log(level, "   staging = ", staging);
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
        semaphore = Semaphore::create(device, VK_PIPELINE_STAGE_TRANSFER_BIT);
    }

    VkResult result = VK_SUCCESS;

    // allocate staging buffer if required
    VkDeviceSize totalSize = _dynamicDataTotalSize + _dynamicImageTotalSize;
    if (!staging || staging->size < totalSize)
    {
        VkMemoryPropertyFlags stagingMemoryPropertiesFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        staging = vsg::createBufferAndMemory(device, totalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, stagingMemoryPropertiesFlags);

        auto stagingMemory = staging->getDeviceMemory(deviceID);
        buffer_data = nullptr;
        result = stagingMemory->map(staging->getMemoryOffset(deviceID), staging->size, 0, &buffer_data);
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

    // transfer the modifed BufferInfo and ImageInfo
    _transferBufferInfos(vk_commandBuffer, frame, offset);
    _transferImageInfos(vk_commandBuffer, frame, offset);

    vkEndCommandBuffer(vk_commandBuffer);

    // if no regions to copy have been found then commandBuffer will be empty so no need to submit it to queue and use the assocaited single semaphore
    if (offset > 0)
    {
        // submit the transfer commands
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = nullptr;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vk_commandBuffer;

        submitInfo.signalSemaphoreCount = 1;
        VkSemaphore vk_transferCompletedSemaphore = *semaphore;
        submitInfo.pSignalSemaphores = &vk_transferCompletedSemaphore;

        result = transferQueue->submit(submitInfo);
        if (result != VK_SUCCESS) return result;

        currentTransferCompletedSemaphore = semaphore;
    }
    else
    {
        log(level, "Nothing to submit");
    }

    return VK_SUCCESS;
}
