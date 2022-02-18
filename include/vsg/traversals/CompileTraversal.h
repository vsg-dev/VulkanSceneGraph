#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Object.h>
#include <vsg/nodes/Bin.h>
#include <vsg/nodes/Group.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/ResourceHints.h>
#include <vsg/viewer/Window.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/Fence.h>
#include <vsg/vk/ResourceRequirements.h>

namespace vsg
{

    class VSG_DECLSPEC CompileTraversal : public Inherit<Visitor, CompileTraversal>
    {
    public:
        CompileTraversal() {}
        CompileTraversal(const CompileTraversal& ct);
        explicit CompileTraversal(ref_ptr<Device> device, const ResourceRequirements& resourceRequirements = {});
        explicit CompileTraversal(ref_ptr<Window> window, ref_ptr<ViewportState> viewport = {}, const ResourceRequirements& resourceRequirements = {});
        explicit CompileTraversal(Viewer& viewer, const ResourceRequirements& resourceRequirements = {});

        /// list Context that Vulkan objects should be compiled for.
        std::list<ref_ptr<Context>> contexts;

        /// add a compile Context for device
        void add(ref_ptr<Device> device, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for Window and associated viewport.
        void add(ref_ptr<Window> window, ref_ptr<ViewportState> viewport = {}, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for View
        void add(ref_ptr<Window> window, ref_ptr<View> view, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for all the Views assigned to a Viewer
        void add(Viewer& viewer, const ResourceRequirements& resourceRequirements = {});

        virtual bool record();
        virtual void waitForCompletion();

        /// convinience method that compiles a object/subgraph
        template<typename T>
        void compile(T object, bool wait = true)
        {
            object->accept(*this);
            if (record() && wait) waitForCompletion();
        }

        // implement compile of relevant nodes in the viewer/scene graph
        void apply(Object& object) override;
        void apply(Command& command) override;
        void apply(Commands& commands) override;
        void apply(StateGroup& stateGroup) override;
        void apply(Geometry& geometry) override;
        void apply(CommandGraph& commandGraph) override;
        void apply(RenderGraph& renderGraph) override;
        void apply(View& view) override;

    protected:
        ~CompileTraversal();
    };
    VSG_type_name(vsg::CompileTraversal);

} // namespace vsg
