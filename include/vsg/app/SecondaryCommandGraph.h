#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/CommandGraph.h>

namespace vsg
{

    // forward declare
    class ExecuteCommands;

    /// SecondaryCommandGraph is a specialization of CommandGraph that provides Vulkan secondary command buffer support.
    class VSG_DECLSPEC SecondaryCommandGraph : public Inherit<CommandGraph, SecondaryCommandGraph>
    {
    public:
        SecondaryCommandGraph(ref_ptr<Device> in_device, int family);
        explicit SecondaryCommandGraph(ref_ptr<Window> in_window, ref_ptr<Node> child = {}, uint32_t in_subpass = 0);

        uint32_t subpass = 0;
        VkBool32 occlusionQueryEnable = VK_FALSE;
        VkQueryControlFlags queryFlags = 0;
        VkQueryPipelineStatisticFlags pipelineStatistics = 0;

        VkCommandBufferLevel level() const override;
        void reset() override;
        void record(CommandBuffers& recordedCommandBuffers, ref_ptr<FrameStamp> frameStamp = {}, ref_ptr<DatabasePager> databasePager = {}) override;

    protected:
        virtual ~SecondaryCommandGraph();

        friend ExecuteCommands;

        void _connect(ExecuteCommands* executeCommand);
        void _disconnect(ExecuteCommands* executeCommand);

        std::vector<ExecuteCommands*> _executeCommands;
    };
    VSG_type_name(vsg::SecondaryCommandGraph);

    using SecondaryCommandGraphs = std::vector<ref_ptr<SecondaryCommandGraph>>;

    /// convenience function that sets up secondaryCommandGraph to render the specified scene graph from the specified Camera view
    extern VSG_DECLSPEC ref_ptr<SecondaryCommandGraph> createSecondaryCommandGraphForView(ref_ptr<Window> window, ref_ptr<Camera> camera, ref_ptr<Node> scenegraph, uint32_t subpass, bool assignHeadlight = true);

} // namespace vsg
