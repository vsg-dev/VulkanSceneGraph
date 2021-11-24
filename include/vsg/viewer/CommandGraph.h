#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Export.h>
#include <vsg/nodes/Bin.h>
#include <vsg/nodes/Group.h>
#include <vsg/viewer/Camera.h>
#include <vsg/viewer/Window.h>
#include <vsg/vk/CommandBuffer.h>

namespace vsg
{

    // forward declare
    class ExecuteCommands;

    class VSG_DECLSPEC CommandGraph : public Inherit<Group, CommandGraph>
    {
    public:
        CommandGraph(Device* in_device, int family);
        explicit CommandGraph(Window* in_window);

        // settings, configure at construction time
        ref_ptr<Window> window;
        ref_ptr<Device> device;
        ref_ptr<Camera> camera;

        int queueFamily = -1;
        int presentFamily = -1;
        uint32_t maxSlot = 2;

        VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        uint32_t subpass = 0;
        VkBool32 occlusionQueryEnable = VK_FALSE;
        VkQueryControlFlags queryFlags = 0;
        VkQueryPipelineStatisticFlags pipelineStatistics = 0;

        ref_ptr<RecordTraversal> recordTraversal;

        void reset();

        virtual void record(CommandBuffers& recordedCommandBuffers, ref_ptr<FrameStamp> frameStamp = {}, ref_ptr<DatabasePager> databasePager = {});

    protected:
        virtual ~CommandGraph();

        void _connect(ExecuteCommands* executeCommand);
        void _disconnect(ExecuteCommands* executeCommand);

        CommandBuffers _commandBuffers; // assign one per index? Or just use round robin, each has a CommandPool

        std::vector<ExecuteCommands*> _executeCommands;

        friend ExecuteCommands;
    };
    VSG_type_name(vsg::CommandGraph);

    using CommandGraphs = std::vector<ref_ptr<CommandGraph>>;

    /// convenience function that sets up RenderGraph inside primary CommandGraph to render the specified scene graph from the specified Camera view
    extern VSG_DECLSPEC ref_ptr<CommandGraph> createCommandGraphForView(ref_ptr<Window> window, ref_ptr<Camera> camera, ref_ptr<Node> scenegraph, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

    /// convenience function that sets up secondaryCommandGraph to render the specified scene graph from the specified Camera view
    extern VSG_DECLSPEC ref_ptr<CommandGraph> createSecondaryCommandGraphForView(ref_ptr<Window> window, ref_ptr<Camera> camera, ref_ptr<Node> scenegraph, uint32_t subpass);

} // namespace vsg
