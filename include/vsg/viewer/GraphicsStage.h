#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Stage.h>

#include <vsg/io/DatabasePager.h>

#include <vsg/viewer/Camera.h>

namespace vsg
{

    class VSG_DECLSPEC GraphicsStage : public Inherit<Stage, GraphicsStage>
    {
    public:
        GraphicsStage(ref_ptr<Node> commandGraph, ref_ptr<Camera> camera = ref_ptr<Camera>());

        ref_ptr<Camera> _camera;
        ref_ptr<Node> _commandGraph;
        ref_ptr<dmat4Value> _projMatrix;
        ref_ptr<dmat4Value> _viewMatrix;
        ref_ptr<ViewportState> _viewport;

        ref_ptr<DatabasePager> databasePager;

        VkExtent2D _extent2D;
        uint32_t _maxSlot = 2;

        void populateCommandBuffer(CommandBuffer* commandBuffer, Framebuffer* framebuffer, RenderPass* renderPass, ImageView* imageView, const VkExtent2D& extent2D, const VkClearColorValue& clearColor, ref_ptr<FrameStamp> frameStamp) override;
    };

} // namespace vsg
