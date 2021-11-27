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

    // RecordAndSubmitTask
    class RecordAndSubmitTask : public Inherit<Object, RecordAndSubmitTask>
    {
    public:
        explicit RecordAndSubmitTask(Device* device, uint32_t numBuffers = 3);

        virtual VkResult submit(ref_ptr<FrameStamp> frameStamp = {});

        virtual VkResult start();
        virtual VkResult record(CommandBuffers& recordedCommandBuffers, ref_ptr<FrameStamp> frameStamp);
        virtual VkResult finish(CommandBuffers& recordedCommandBuffers);

        Windows windows;
        Semaphores waitSemaphores;
        CommandGraphs commandGraphs; // assign in application setup
        Semaphores signalSemaphores; // connect to Presentation.waitSemaphores

        /// advance the currentFrameIndex
        void advance();

        /// return the fence index value for relativeFrameIndex where 0 is current frame, 1 is previous frame etc.
        size_t index(size_t relativeFrameIndex = 0) const { return relativeFrameIndex < _indices.size() ? _indices[relativeFrameIndex] : _indices.size(); }

        /// fence() and fence(0) return the Fence for the frame currently being rendered, fence(1) return the previous frame's Fence etc.
        Fence* fence(size_t relativeFrameIndex = 0)
        {
            size_t i = index(relativeFrameIndex);
            return i < _fences.size() ? _fences[i].get() : nullptr;
        }

        ref_ptr<Queue> queue; // assign in application for GraphicsQueue from device

        ref_ptr<DatabasePager> databasePager;

    protected:
        size_t _currentFrameIndex;
        std::vector<size_t> _indices;
        std::vector<ref_ptr<Fence>> _fences;
    };
    VSG_type_name(vsg::RecordAndSubmitTask);

} // namespace vsg
