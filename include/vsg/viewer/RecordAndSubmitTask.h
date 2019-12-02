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

#include <vsg/viewer/Window.h>
#include <vsg/viewer/CommandGraph.h>

namespace vsg
{

    // RecordAndSubmitTask
    class RecordAndSubmitTask : public Inherit<Object, RecordAndSubmitTask>
    {
    public:

        // Need to add FrameStamp?
        VkResult submit(ref_ptr<FrameStamp> frameStamp);

        using CommandGraphs = std::vector<ref_ptr<CommandGraph>>;

        Windows windows;
        Semaphores waitSemaphores; //
        CommandGraphs commandGraphs; // assign in application setup
        Semaphores signalSemaphores; // connect to Presentation.waitSemaphores

        ref_ptr<DatabasePager> databasePager;
        ref_ptr<Queue> queue; // assign in application for GraphicsQueue from device
    };

} // namespace vsg
