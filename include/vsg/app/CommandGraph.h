#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/Camera.h>
#include <vsg/app/Window.h>
#include <vsg/core/Export.h>
#include <vsg/nodes/Bin.h>
#include <vsg/nodes/Group.h>
#include <vsg/vk/CommandBuffer.h>

namespace vsg
{

    /// CommandGraph is a group node that sits at the top of the scene graph and manages the recording of it's subgraph to Vulkan command buffers.
    class VSG_DECLSPEC CommandGraph : public Inherit<Group, CommandGraph>
    {
    public:
        CommandGraph(ref_ptr<Device> in_device, int family);
        explicit CommandGraph(ref_ptr<Window> in_window, ref_ptr<Node> child = {});

        // Settings, configure at construction time.
        // When either window or framebuffer is assigned, if framebuffer is set then it takes precedence, if not the appropriate window's framebuffer is used.
        ref_ptr<Framebuffer> framebuffer;
        ref_ptr<Window> window;
        ref_ptr<Device> device;
        ref_ptr<Camera> camera;

        int queueFamily = -1;
        int presentFamily = -1;
        uint32_t maxSlot = 2;

        ref_ptr<RecordTraversal> recordTraversal;

        virtual VkCommandBufferLevel level() const;
        virtual void reset();
        virtual void record(CommandBuffers& recordedCommandBuffers, ref_ptr<FrameStamp> frameStamp = {}, ref_ptr<DatabasePager> databasePager = {});

    protected:
        virtual ~CommandGraph();

        CommandBuffers _commandBuffers; // assign one per index? Or just use round robin, each has a CommandPool
    };
    VSG_type_name(vsg::CommandGraph);

    using CommandGraphs = std::vector<ref_ptr<CommandGraph>>;

    /// convenience function that sets up RenderGraph inside primary CommandGraph to render the specified scene graph from the specified Camera view
    extern VSG_DECLSPEC ref_ptr<CommandGraph> createCommandGraphForView(ref_ptr<Window> window, ref_ptr<Camera> camera, ref_ptr<Node> scenegraph, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE, bool assignHeadlight = true);

} // namespace vsg
