#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/CompileTraversal.h>
#include <vsg/threading/OperationQueue.h>

namespace vsg
{

    /// CompileResult struct encapsulates the results of compile traversal.
    /// Used to help guide further operations done with the compiled subgraph.
    struct CompileResult
    {
        VkResult result = VK_INCOMPLETE;
        uint32_t maxSlot = 0;
        bool containsPagedLOD = false;
        ResourceRequirements::Views views;
        ResourceRequirements::DynamicData earlyDynamicData;
        ResourceRequirements::DynamicData lateDynamicData;

        explicit operator bool() const noexcept { return result == VK_SUCCESS; }
    };

    /// ComppileManager is a helper class that compiles subgraphs for the window/framebuffer associated with the CompileManager.
    class VSG_DECLSPEC CompileManager : public Inherit<Object, CompileManager>
    {
    public:
        CompileManager(Viewer& viewer, ref_ptr<ResourceHints> hints);

        /// add a compile Context for device
        void add(ref_ptr<Device> device, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for Window and associated viewport.
        void add(Window& window, ref_ptr<ViewportState> viewport = {}, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for View
        void add(Window& window, ref_ptr<View> view, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for Framebuffer and associated View
        void add(Framebuffer& framebuffer, ref_ptr<View> view, const ResourceRequirements& resourceRequirements = {});

        /// add a compile Context for all the Views assigned to a Viewer
        void add(const Viewer& viewer, const ResourceRequirements& resourceRequirements = {});

        using ContextSelectionFunction = std::function<bool(vsg::Context&)>;

        /// compile object
        CompileResult compile(ref_ptr<Object> object, ContextSelectionFunction contextSelection = {});

    protected:
        using CompileTraversals = ThreadSafeQueue<ref_ptr<CompileTraversal>>;
        size_t numCompileTraversals = 0;
        ref_ptr<CompileTraversals> compileTraversals;

        CompileTraversals::container_type takeCompileTraversals(size_t count);
    };
    VSG_type_name(vsg::CompileManager);

} // namespace vsg
