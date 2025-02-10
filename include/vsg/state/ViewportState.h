#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/GraphicsPipeline.h>

namespace vsg
{
    using Viewports = std::vector<VkViewport>;
    using Scissors = std::vector<VkRect2D>;

    /// ViewportState encapsulates VkPipelineViewportStateCreateInfo settings passed when setting up GraphicsPipeline
    class VSG_DECLSPEC ViewportState : public Inherit<GraphicsPipelineState, ViewportState>
    {
    public:
        ViewportState();
        ViewportState(const ViewportState& vs);

        /// Create ViewportState containing a single Viewport and Scissor pair with specified extent located at origin (x, y = {0,0}). Typically used for convenience when rendering to a whole window.
        explicit ViewportState(const VkExtent2D& extent);

        /// Create ViewportState containing a single Viewport and Scissor pair with specified position and extent
        ViewportState(int32_t x, int32_t y, uint32_t width, uint32_t height);

        /// VkPipelineViewportStateCreateInfo settings
        Viewports viewports;
        Scissors scissors;

        /// set to a single Viewport and Scissor pair with specified extent
        void set(int32_t x, int32_t y, uint32_t width, uint32_t height);

        /// get or create the first Viewport
        VkViewport& getViewport();

        /// get or create the first Scissor
        VkRect2D& getScissor();

        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;
        void apply(Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const override;

        /// enable ViewportState to be recorded via vkCmdSetScissor and vkCmdSetViewport
        void record(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~ViewportState();
    };
    VSG_type_name(vsg::ViewportState);

} // namespace vsg
