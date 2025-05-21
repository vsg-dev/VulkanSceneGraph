#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/state/ImageInfo.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/ResourceRequirements.h>
#include <vsg/vk/Semaphore.h>

namespace vsg
{

    /// TransferTask manages a collection of dynamically updated vsg::Data associated with GPU memory of vsg::BufferInfo and vsg::ImageInfo.
    /// During the viewer.compile(..) traversal the collection of dynamic data that has dataVariance of DYNAMIC_DATA* is assigned to the appropriate TransferTask
    /// and then each new frame that collection of data is checked to see if the modification count has changed, if it has that data is copied to the associated BufferInfo/ImageInfo.
    /// vsg::Data that are orphaned so the TransferTask has the only remaining reference to them are automatically removed.
    class VSG_DECLSPEC TransferTask : public Inherit<Object, TransferTask>
    {
    public:
        explicit TransferTask(Device* in_device, uint32_t numBuffers = 3);

        struct TransferResult
        {
            VkResult result = VK_SUCCESS;
            ref_ptr<Semaphore> dataTransferredSemaphore;
        };

        enum TransferMask
        {
            TRANSFER_NONE = 0,
            TRANSFER_BEFORE_RECORD_TRAVERSAL = 1 << 0,
            TRANSFER_AFTER_RECORD_TRAVERSAL = 1 << 1,
            TRANSFER_ALL = TRANSFER_BEFORE_RECORD_TRAVERSAL | TRANSFER_AFTER_RECORD_TRAVERSAL
        };

        /// transfer any vsg::Data entries that have been updated to the associated GPU memory.
        virtual TransferResult transferData(TransferMask transferMask);

        virtual bool containsDataToTransfer(TransferMask transferMask) const;

        ref_ptr<Device> device;

        void assign(const ResourceRequirements::DynamicData& dynamicData);
        void assign(const BufferInfoList& bufferInfoList);
        void assign(const ImageInfoList& imageInfoList);

        ref_ptr<Queue> transferQueue;

        /// minimum size to use when allocating staging buffers.
        VkDeviceSize minimumStagingBufferSize = 16 * 1024 * 1024;

        // allocation chunk size
        VkDeviceSize stagingBufferSizeChunkSize = 64 * 1024;

        /// hook for assigning Instrumentation to enable profiling of record traversal.
        ref_ptr<Instrumentation> instrumentation;

        /// control for the level of debug information emitted by the TransferTask
        Logger::Level level = Logger::LOGGER_DEBUG;

        void assignTransferConsumedCompletedSemaphore(TransferMask transferMask, ref_ptr<Semaphore> semaphore);

    protected:
        using OffsetBufferInfoMap = std::map<VkDeviceSize, ref_ptr<BufferInfo>>;
        using BufferMap = std::map<ref_ptr<Buffer>, OffsetBufferInfoMap>;

        mutable std::mutex _mutex;

        struct TransferBlock : public Inherit<Object, TransferBlock>
        {
            ref_ptr<CommandBuffer> transferCommandBuffer;
            ref_ptr<Fence> fence;
            std::vector<ref_ptr<Buffer>> stagingBuffers;
            std::vector<std::vector<VkDeviceSize>> stagingOffsets;
            std::vector<void*> buffer_data;
            std::vector<VkBufferCopy> copyRegions;
            bool waitOnFence = false;
        };

        struct DataToCopy
        {
            std::string name;
            BufferMap dataMap;
            std::set<ref_ptr<ImageInfo>> imageInfoSet;

            uint32_t frameIndex = 0;
            std::vector<ref_ptr<TransferBlock>> frames;

            VkDeviceSize dataTotalRegions = 0;

            ref_ptr<Semaphore> transferCompleteSemaphore;
            ref_ptr<Semaphore> transferConsumerCompletedSemaphore;

            bool requiresCopy(uint32_t deviceID) const;
            bool containsDataToTransfer() const { return !dataMap.empty() || !imageInfoSet.empty(); }
        };

        DataToCopy _earlyDataToCopy;
        DataToCopy _lateDataToCopy;

        size_t _bufferCount;

        TransferResult _transferData(DataToCopy& dataToCopy);

        void _transferBufferInfos(DataToCopy& dataToCopy, VkCommandBuffer vk_commandBuffer, TransferBlock& frame, size_t stagingBufferOffsetBegin, size_t regionOffsetBegin);

        void _transferImageInfos(DataToCopy& dataToCopy, VkCommandBuffer vk_commandBuffer, TransferBlock& frame, size_t stagingBufferOffsetBegin, size_t regionOffsetBegin);
        void _transferImageInfo(VkCommandBuffer vk_commandBuffer, TransferBlock& frame, size_t stagingBufferOffset, size_t regionOffset, ImageInfo& imageInfo);
    };
    VSG_type_name(vsg::TransferTask);

} // namespace vsg
