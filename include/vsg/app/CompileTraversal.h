#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/Window.h>
#include <vsg/core/Object.h>
#include <vsg/nodes/Bin.h>
#include <vsg/nodes/Group.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/ResourceHints.h>
#include <vsg/utils/Instrumentation.h>
#include <vsg/vk/CommandPool.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/Fence.h>
#include <vsg/vk/ResourceRequirements.h>

namespace vsg
{

    /// CompileTraversal traverses a scene graph and invokes all the StateCommand/Command::compile(..) methods to create all Vulkan objects, allocate GPU memory and transfer data to GPU.
    class VSG_DECLSPEC CompileTraversal : public Inherit<Visitor, CompileTraversal>
    {
    public:
        CompileTraversal() { overrideMask = vsg::MASK_ALL; }
        CompileTraversal(const CompileTraversal& ct);
        explicit CompileTraversal(ref_ptr<Device> device, const ResourceRequirements& resourceRequirements = {});
        explicit CompileTraversal(Window& window, ref_ptr<ViewportState> viewport = {}, const ResourceRequirements& resourceRequirements = {});
        explicit CompileTraversal(const Viewer& viewer, const ResourceRequirements& resourceRequirements = {});

        /// specification of the queue to use
        VkQueueFlags queueFlags = VK_QUEUE_GRAPHICS_BIT;
        uint32_t queueFamilyIndex = 1;

        /// list of Context that Vulkan objects should be compiled for.
        std::list<ref_ptr<Context>> contexts;

        /// Hook for assigning Instrumentation to enable profiling
        ref_ptr<Instrumentation> instrumentation;

        /// add a compile Context for device
        void add(ref_ptr<Device> device, ref_ptr<TransferTask> transferTask, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for device
        void add(ref_ptr<Device> device, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for Window and associated viewport.
        void add(Window& window, ref_ptr<TransferTask> transferTask, ref_ptr<ViewportState> viewport = {}, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for Window and associated viewport.
        void add(Window& window, ref_ptr<ViewportState> viewport = {}, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for Window and associated View
        void add(Window& window, ref_ptr<TransferTask> transferTask, ref_ptr<View> view, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for Window and associated View
        void add(Window& window, ref_ptr<View> view, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for Framebuffer and associated View
        void add(Framebuffer& framebuffer, ref_ptr<TransferTask> transferTask, ref_ptr<View> view, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for Framebuffer and associated View
        void add(Framebuffer& framebuffer, ref_ptr<View> view, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for all the Views assigned to a Viewer
        void add(const Viewer& viewer, const ResourceRequirements& resourceRequirements = {});

        /// assign Instrumentation to all Context
        void assignInstrumentation(ref_ptr<Instrumentation> in_instrumentation);

        Instrumentation* getInstrumentation() override { return instrumentation.get(); }

        virtual bool record();
        virtual void waitForCompletion();

        /// convenience method that compiles an object/subgraph
        template<typename T>
        void compile(T object, bool wait = true)
        {
            object->accept(*this);
            if (record() && wait) waitForCompletion();
        }

        // implement compile of relevant nodes in the scene graph
        void apply(Object& object) override;
        void apply(Compilable& node) override;
        void apply(Commands& commands) override;
        void apply(Geometry& geometry) override;
        void apply(CommandGraph& commandGraph) override;
        void apply(SecondaryCommandGraph& secondaryCommandGraph) override;
        void apply(RenderGraph& renderGraph) override;
        void apply(View& view) override;

    protected:
        ~CompileTraversal();

        void add(ref_ptr<Context> context, Framebuffer& framebuffer, ref_ptr<TransferTask> transferTask, ref_ptr<View> view, const ResourceRequirements& resourceRequirements);
        void addViewDependentState(ViewDependentState& viewDependentState, ref_ptr<Device> device, ref_ptr<TransferTask> transferTask, const ResourceRequirements& resourceRequirements);
    };
    VSG_type_name(vsg::CompileTraversal);

} // namespace vsg
