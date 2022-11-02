/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/RenderGraph.h>
#include <vsg/app/View.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/Bin.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/State.h>

using namespace vsg;

RenderGraph::RenderGraph() :
    windowResizeHandler(WindowResizeHandler::create())
{
}

RenderGraph::RenderGraph(ref_ptr<Window> in_window, ref_ptr<View> in_view) :
    window(in_window),
    windowResizeHandler(WindowResizeHandler::create())
{

    if (in_view)
    {
        addChild(in_view);
    }

    previous_extent = window->extent2D();

    if (in_view && in_view->camera && in_view->camera->viewportState)
    {
        renderArea = in_view->camera->getRenderArea();
    }
    else
    {
        renderArea.offset = {0, 0};
        renderArea.extent = window->extent2D();
    }

    // set up the clearValues based on the RenderPass's attachments.
    setClearValues(window->clearColor(), VkClearDepthStencilValue{0.0f, 0});
}

RenderPass* RenderGraph::getRenderPass()
{
    if (framebuffer)
    {
        return framebuffer->getRenderPass();
    }
    else if (window)
    {
        return window->getOrCreateRenderPass();
    }
    return nullptr;
}

void RenderGraph::setClearValues(VkClearColorValue clearColor, VkClearDepthStencilValue clearDepthStencil)
{
    auto renderPass = getRenderPass();
    if (!renderPass) return;

    auto& attachments = renderPass->attachments;
    clearValues.resize(attachments.size());
    for (size_t i = 0; i < attachments.size(); ++i)
    {
        if (attachments[i].finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            clearValues[i].depthStencil = clearDepthStencil;
        }
        else
        {
            clearValues[i].color = clearColor;
        }
    }
}

VkExtent2D RenderGraph::getExtent() const
{
    if (framebuffer)
        return VkExtent2D{framebuffer->width(), framebuffer->height()};
    else if (window)
        return window->extent2D();
    else
        return VkExtent2D{invalid_dimension, invalid_dimension};
}

void RenderGraph::accept(RecordTraversal& recordTraversal) const
{
    auto extent = getExtent();
    if (previous_extent.width == invalid_dimension || previous_extent.height == invalid_dimension || !windowResizeHandler)
    {
        previous_extent = extent;
    }
    else if (previous_extent.width != extent.width || previous_extent.height != extent.height)
    {
        auto this_renderGraph = const_cast<RenderGraph*>(this);
        this_renderGraph->resized();
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    if (framebuffer)
    {
        renderPassInfo.renderPass = *(framebuffer->getRenderPass());
        renderPassInfo.framebuffer = *(framebuffer);
    }
    else
    {
        size_t imageIndex = window->imageIndex();
        if (imageIndex >= window->numFrames()) return;

        renderPassInfo.renderPass = *(window->getRenderPass());
        renderPassInfo.framebuffer = *(window->framebuffer(imageIndex));
    }

    renderPassInfo.renderArea = renderArea;

#if 0
    debug("RenaderGraph() ", this, " renderArea.offset.x = ", renderArea.offset.x, ", renderArea.offset.y = ", renderArea.offset.y
                , ", renderArea.extent.width = ", renderArea.extent.width, ", renderArea.extent.height = ", renderArea.extent.height);
#endif

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    VkCommandBuffer vk_commandBuffer = *(recordTraversal.getState()->_commandBuffer);
    vkCmdBeginRenderPass(vk_commandBuffer, &renderPassInfo, contents);

    // traverse the command buffer to place the commands into the command buffer.
    traverse(recordTraversal);

    vkCmdEndRenderPass(vk_commandBuffer);
}

void RenderGraph::resized()
{
    if (!windowResizeHandler) return;
    if (!window && !framebuffer) return;

    auto renderPass = getRenderPass();
    if (!renderPass) return;

    auto device = renderPass->device;

    if (!windowResizeHandler->context) windowResizeHandler->context = vsg::Context::create(device);

    auto extent = getExtent();

    windowResizeHandler->context->commandPool = nullptr;
    windowResizeHandler->context->renderPass = renderPass;
    windowResizeHandler->renderArea = renderArea;
    windowResizeHandler->previous_extent = previous_extent;
    windowResizeHandler->new_extent = extent;
    windowResizeHandler->visited.clear();

    if (renderPass->maxSamples != VK_SAMPLE_COUNT_1_BIT)
    {
        windowResizeHandler->context->overridePipelineStates.emplace_back(vsg::MultisampleState::create(renderPass->maxSamples));
    }

    // make sure the device is idle before we recreate any Vulkan objects
    vkDeviceWaitIdle(*(device));

    traverse(*windowResizeHandler);

    windowResizeHandler->scale_rect(renderArea);

    previous_extent = extent;
}

ref_ptr<RenderGraph> vsg::createRenderGraphForView(ref_ptr<Window> window, ref_ptr<Camera> camera, ref_ptr<Node> scenegraph, VkSubpassContents contents, bool assignHeadlight)
{
    // set up the view
    auto view = View::create(camera);
    if (assignHeadlight) view->addChild(createHeadlight());
    if (scenegraph) view->addChild(scenegraph);

    // set up the render graph
    auto renderGraph = RenderGraph::create(window, view);
    renderGraph->contents = contents;

    return renderGraph;
}
