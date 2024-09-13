#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/CommandGraph.h>
#include <vsg/app/TransferTask.h>
#include <vsg/app/Window.h>
#include <vsg/io/DatabasePager.h>
#include <vsg/nodes/Group.h>
#include <vsg/utils/Instrumentation.h>
#include <vsg/vk/CommandBuffer.h>

namespace vsg
{

    /// RecordAndSubmitTask manages the recording of its list of CommandGraph to CommandBuffer which are then submitted to the associated vulkan Queue.
    class VSG_DECLSPEC RecordAndSubmitTask : public Inherit<Object, RecordAndSubmitTask>
    {
    public:
        explicit RecordAndSubmitTask(Device* in_device, uint32_t numBuffers = 3);

        virtual VkResult submit(ref_ptr<FrameStamp> frameStamp = {});

        virtual VkResult start();
        virtual VkResult record(ref_ptr<RecordedCommandBuffers> recordedCommandBuffers, ref_ptr<FrameStamp> frameStamp);
        virtual VkResult finish(ref_ptr<RecordedCommandBuffers> recordedCommandBuffers);

        ref_ptr<Device> device;
        Windows windows;
        Semaphores waitSemaphores;   // assign in application setup
        CommandGraphs commandGraphs; // assign in application setup
        Semaphores signalSemaphores; // connect to Presentation.waitSemaphores

        Semaphores transientWaitSemaphores; // assign per frame and then cleared by finish(), assumed reference to semaphores assigned are retained elsewhere to pevert deletion while still in use.
        Semaphores transientSignalSemaphores; // assign per frame and then cleared by finish(), assumed reference to semaphores assigned are retained elsewhere to pevert deletion while still in use.

        ref_ptr<TransferTask> transferTask; // data is transferred for this frame

        ref_ptr<Semaphore> earlyTransferConsumerCompletedSemaphore;
        bool earlyDataTransferred = false;

        ref_ptr<Semaphore> lateTransferConsumerCompletedSemaphore;
        bool lateDataTransferred = false;

        /// advance the currentFrameIndex
        void advance();

        /// return the fence index value for relativeFrameIndex where 0 is current frame, 1 is previous frame etc.
        size_t index(size_t relativeFrameIndex = 0) const;

        /// fence() and fence(0) return the Fence for the frame currently being rendered, fence(1) returns the previous frame's Fence etc.
        ref_ptr<Fence> fence(size_t relativeFrameIndex = 0);

        ref_ptr<Queue> queue;

        ref_ptr<DatabasePager> databasePager;

        /// hook for assigning Instrumentation to enable profiling of record traversal.
        ref_ptr<Instrumentation> instrumentation;

        /// Convenience method for assigning Instrumentation to the viewer and any associated objects.
        void assignInstrumentation(ref_ptr<Instrumentation> in_instrumentation);

    protected:
        size_t _currentFrameIndex;
        std::vector<size_t> _indices;
        std::vector<ref_ptr<Fence>> _fences;
    };
    VSG_type_name(vsg::RecordAndSubmitTask);

    using RecordAndSubmitTasks = std::vector<ref_ptr<RecordAndSubmitTask>>;

    /// update RecordAndSubmitTask data structures to match the needs of newly compiled subgraphs
    extern VSG_DECLSPEC void updateTasks(RecordAndSubmitTasks& tasks, ref_ptr<CompileManager> compileManager, const CompileResult& compileResult);

} // namespace vsg
