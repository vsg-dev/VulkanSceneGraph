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
            ref_ptr<Semaphore> semaphore;
        };

        /// transfer any vsg::Data entries that have been updated to the associated GPU memory.
        virtual TransferResult transferData();

        virtual bool containsDataToTransfer() const;

        ref_ptr<Device> device;

        /// advance the currentTransferBlockIndex
        void advance();

        void assign(const ResourceRequirements::DynamicData& dynamicData);
        void assign(const BufferInfoList& bufferInfoList);
        void assign(const ImageInfoList& imageInfoList);

        ref_ptr<Queue> transferQueue;

        /// minimum size to use when allocating staging buffers.
        VkDeviceSize minimumStagingBufferSize = 16 * 1024 * 1024;

        /// hook for assigning Instrumentation to enable profiling of record traversal.
        ref_ptr<Instrumentation> instrumentation;

        /// control for the level of debug infomation emitted by the TransferTask
        Logger::Level level = Logger::LOGGER_DEBUG;

    protected:
        using OffsetBufferInfoMap = std::map<VkDeviceSize, ref_ptr<BufferInfo>>;
        using BufferMap = std::map<ref_ptr<Buffer>, OffsetBufferInfoMap>;

        size_t index(size_t relativeTransferBlockIndex = 0) const;

        mutable std::mutex _mutex;

        struct TransferBlock : public Inherit<Object, TransferBlock>
        {
            ref_ptr<CommandBuffer> transferCommandBuffer;
            ref_ptr<Semaphore> transferCompleteSemaphore;
            ref_ptr<Buffer> staging;
            void* buffer_data = nullptr;
            std::vector<VkBufferCopy> copyRegions;
        };

        struct DataToCopy
        {
            BufferMap dataMap;
            std::set<ref_ptr<ImageInfo>> imageInfoSet;
            std::vector<ref_ptr<TransferBlock>> frames;

            VkDeviceSize dataTotalRegions = 0;
            VkDeviceSize dataTotalSize = 0;
            VkDeviceSize imageTotalSize = 0;

            bool containsDataToTransfer() const { return !dataMap.empty() || !imageInfoSet.empty(); }
        };

        DataToCopy _earlyDataToCopy;
        DataToCopy _lateDataToCopy;

        size_t _currentTransferBlockIndex;
        std::vector<size_t> _indices;

        TransferResult  _transferData(DataToCopy& dataToCopy);

        void _transferBufferInfos(DataToCopy& dataToCopy, VkCommandBuffer vk_commandBuffer, TransferBlock& frame, VkDeviceSize& offset);

        void _transferImageInfos(DataToCopy& dataToCopy, VkCommandBuffer vk_commandBuffer, TransferBlock& frame, VkDeviceSize& offset);
        void _transferImageInfo(VkCommandBuffer vk_commandBuffer, TransferBlock& frame, VkDeviceSize& offset, ImageInfo& imageInfo);
    };
    VSG_type_name(vsg::TransferTask);

} // namespace vsg
