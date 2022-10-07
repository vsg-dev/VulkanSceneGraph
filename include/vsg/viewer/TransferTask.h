#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Group.h>

#include <vsg/vk/CommandBuffer.h>

#include <vsg/io/DatabasePager.h>

#include <vsg/viewer/CommandGraph.h>
#include <vsg/viewer/Window.h>

namespace vsg
{

    // TransferTask
    class VSG_DECLSPEC TransferTask : public Inherit<Object, TransferTask>
    {
    public:
        explicit TransferTask(Device* in_device, uint32_t numBuffers = 3);

        virtual VkResult transferDynamicData(); // transfer dynamicBufferInfos entries to GPU

        ref_ptr<Device> device;
        Semaphores waitSemaphores;
        Semaphores signalSemaphores;       // connect to Presentation.waitSemaphores

        /// advance the currentFrameIndex
        void advance();

        /// return the fence index value for relativeFrameIndex where 0 is current frame, 1 is previous frame etc.
        size_t index(size_t relativeFrameIndex = 0) const;

        /// fence() and fence(0) return the Fence for the frame currently being rendered, fence(1) return the previous frame's Fence etc.
        Fence* fence(size_t relativeFrameIndex = 0);

        void assign(const BufferInfoList& bufferInfoList);
        void assign(const ImageInfoList& imageInfoList);

        ref_ptr<Queue> transferQueue;
        ref_ptr<Semaphore> currentTransferCompletedSemaphore;

    protected:

        using OffsetBufferInfoMap = std::map<uint32_t, ref_ptr<BufferInfo>>;
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
            ref_ptr<Semaphore> transferCompledSemaphore;
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

    using TransferTasks = std::vector<ref_ptr<TransferTask>>;

} // namespace vsg
