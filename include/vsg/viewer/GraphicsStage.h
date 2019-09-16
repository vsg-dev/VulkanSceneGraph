#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/Viewer/Window.h>

#include <vsg/viewer/Camera.h>

namespace vsg
{
    class VSG_DECLSPEC GraphicsStage : public Inherit<Stage, GraphicsStage>
    {
    public:
        GraphicsStage(ref_ptr<Node> commandGraph, ref_ptr<Camera> camera = ref_ptr<Camera>());

        ref_ptr<Camera> _camera;
        ref_ptr<Node> _commandGraph;
        vsg::ref_ptr<vsg::mat4Value> _projMatrix;
        vsg::ref_ptr<vsg::mat4Value> _viewMatrix;
        vsg::ref_ptr<ViewportState> _viewport;

        VkExtent2D _extent2D;
        uint32_t _maxSlot = 2;

        void populateCommandBuffer(CommandBuffer* commandBuffer, Framebuffer* framebuffer, RenderPass* renderPass, const VkExtent2D& extent2D, const VkClearColorValue& clearColor) override;
    };

    class VSG_DECLSPEC OffscreenGraphicsStage : public Inherit<GraphicsStage, OffscreenGraphicsStage>
    {
    public:
        OffscreenGraphicsStage(Device* device, ref_ptr<Node> commandGraph, ref_ptr<Camera> camera, VkExtent2D dimensions);

        ref_ptr<Image> _colorImage;
        ref_ptr<ImageView> _colorImageView;
        ref_ptr<DeviceMemory> _colorImageMemory;

        ref_ptr<Image> _depthImage;
        ref_ptr<ImageView> _depthImageView;
        ref_ptr<DeviceMemory> _depthImageMemory;

        ref_ptr<Framebuffer> _frameBuffer;
        ref_ptr<RenderPass> _renderPass;
        ref_ptr<Semaphore> _semaphore;

        void populateCommandBuffer(CommandBuffer* commandBuffer, Framebuffer* framebuffer, RenderPass* renderPass, const VkExtent2D& extent2D, const VkClearColorValue& clearColor) override;
    };

} // namespace vsg
