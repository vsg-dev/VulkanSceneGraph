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

namespace vsg
{
    class RenderGraph : public Inherit<Group, RenderGraph>
    {
    public:
        RenderGraph();

        using Group::accept;

        void accept(RecordTraversal& dispatchTraversal) const override;

        ref_ptr<Camera> camera; // camera that the trackball controls

        ref_ptr<Window> window;
        VkRect2D renderArea; // viewport dimensions

        ref_ptr<RenderPass> renderPass;   // If not set, use window's.
        ref_ptr<Framebuffer> framebuffer; // If not set, use window's.

        RenderPass* getRenderPass()
        {
            if (renderPass)
            {
                return renderPass;
            }
            else
            {
                return window->getOrCreateRenderPass();
            }
        }
        using ClearValues = std::vector<VkClearValue>;
        ClearValues clearValues; // initialize window colour and depth/stencil

        // windopw extent at previous frame
        const uint32_t invalid_dimension = std::numeric_limits<uint32_t>::max();
        mutable VkExtent2D previous_extent = VkExtent2D{invalid_dimension, invalid_dimension};
    };
} // namespace vsg
