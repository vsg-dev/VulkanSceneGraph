#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/CommandGraph.h>
#include <vsg/app/Window.h>
#include <vsg/io/DatabasePager.h>
#include <vsg/nodes/Group.h>
#include <vsg/vk/CommandBuffer.h>

namespace vsg
{

    /// TransferTask manages a collection of dynamically updated vsg::Data associated with GPU memory associated vsg::BufferInfo and vsg::ImageInfo.
    /// During the viewer.compile(..) traversal the collection of dynamic data that has dataVariance of DYNAMIC_DATA* is assigned to the appropriate TransferTask
    /// and then each new frame that collection of data is checked to see if the modification count has changed, if it has that data is copied to the associated BufferInfo/ImageInfo.
    /// vsg::Data that are orphaned so the TransferTask has the only remaining reference to them are automatically removed.
    class VSG_DECLSPEC TransferTask : public Inherit<Object, TransferTask>
    {
    public:
        explicit TransferTask(Device* in_device, uint32_t numBuffers = 3);

        /// transfer any vsg::Data entries, that have been updated, to have the associated GPU memory.
        virtual VkResult transferDynamicData();

        virtual bool containsDataToTransfer() const;

        ref_ptr<Device> device;
        Semaphores waitSemaphores;
        Semaphores signalSemaphores;

        /// advance the currentFrameIndex
        void advance();

        /// return the fence index value for relativeFrameIndex where 0 is current frame, 1 is previous frame etc.
        size_t index(size_t relativeFrameIndex = 0) const;

        /// fence() and fence(0) return the Fence for the frame currently being rendered, fence(1) return the previous frame's Fence etc.
        Fence* fence(size_t relativeFrameIndex = 0);

        void assign(const ResourceRequirements::DynamicData& dynamicData);
        void assign(const BufferInfoList& bufferInfoList);
        void assign(const ImageInfoList& imageInfoList);

        ref_ptr<Queue> transferQueue;
        ref_ptr<Semaphore> currentTransferCompletedSemaphore;

    protected:
        using OffsetBufferInfoMap = std::map<VkDeviceSize, ref_ptr<BufferInfo>>;
        using BufferMap = std::map<ref_ptr<Buffer>, OffsetBufferInfoMap>;

        VkDeviceSize _dynamicDataTotalRegions = 0;
        VkDeviceSize _dynamicDataTotalSize = 0;
        VkDeviceSize _dynamicImageTotalSize = 0;
        BufferMap _dynamicDataMap;
        std::set<ref_ptr<ImageInfo>> _dynamicImageInfoSet;

        size_t _currentFrameIndex;
        std::vector<size_t> _indices;

        struct Frame
        {
            ref_ptr<Fence> fence;
            ref_ptr<CommandBuffer> transferCommandBuffer;
            ref_ptr<Semaphore> transferCompleteSemaphore;
            ref_ptr<Buffer> staging;
            void* buffer_data = nullptr;
            std::vector<VkBufferCopy> copyRegions;
        };

        std::vector<Frame> _frames;

        void _transferBufferInfos(VkCommandBuffer vk_commandBuffer, Frame& frame, VkDeviceSize& offset);

        void _transferImageInfos(VkCommandBuffer vk_commandBuffer, Frame& frame, VkDeviceSize& offset);
        void _transferImageInfo(VkCommandBuffer vk_commandBuffer, Frame& frame, VkDeviceSize& offset, ImageInfo& imageInfo);
    };
    VSG_type_name(vsg::TransferTask);

} // namespace vsg
