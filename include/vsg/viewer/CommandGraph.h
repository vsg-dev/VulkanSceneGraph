#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Group.h>
#include <vsg/viewer/Camera.h>
#include <vsg/viewer/Window.h>
#include <vsg/vk/CommandBuffer.h>

namespace vsg
{

    using CommandGraphs = std::vector<ref_ptr<CommandGraph> >;

    class CommandGraph : public Inherit<Group, CommandGraph>
    {
    public:
        CommandGraph(Device* device, int family);
        CommandGraph(Window* window);

        using Group::accept;

        virtual void record(CommandBuffers& recordedCommandBuffers, ref_ptr<FrameStamp> frameStamp = {}, ref_ptr<DatabasePager> databasePager = {});

        void waitProduction() { _slaveCommandBufferMutex.lock(); }
        ref_ptr<RecordTraversal> recordTraversal;

        Windows windows;

        ref_ptr<Device> _device;
        int _queueFamily = -1;
        int _presentFamily = -1;
        uint32_t _maxSlot = 2;

        VkCommandBufferLevel _commandBuffersLevel;
        uint32_t _subpassIndex;

        mutable CommandBuffers commandBuffers; // assign one per index? Or just use round robin, each has a CommandPool
        ref_ptr<CommandBuffer> lastRecorded;
        std::mutex _slaveCommandBufferMutex; //wait by ExecuteCommands to ensure prod sync

        // setup in Viewer::assignRecordAndSubmitTaskAndPresentation
        ref_ptr<CommandGraph> _masterCommandGraph; // commandgraph embedding this one
        std::shared_ptr<std::mutex> _masterCommandBufferMutex = nullptr; //wait to ensure consumption by primary command buffer

    };

    /// convience function that sets up RenderGraph inside CommandGraph to render the specified scene graph from the speified Camera view
    ref_ptr<CommandGraph> createCommandGraphForView(Window* window, Camera* camera, Node* scenegraph, VkCommandBufferLevel cmdlevel= VK_COMMAND_BUFFER_LEVEL_PRIMARY, uint sub = 0, VkSubpassContents content = VK_SUBPASS_CONTENTS_INLINE);

} // namespace vsg
