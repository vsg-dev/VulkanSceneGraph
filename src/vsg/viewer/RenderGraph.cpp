/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/traversals/RecordTraversal.h>
#include <vsg/viewer/RenderGraph.h>
#include <vsg/viewer/View.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/State.h>

#include <iostream>

using namespace vsg;

RenderGraph::RenderGraph()
{
}

RenderPass* RenderGraph::getRenderPass()
{
    if (framebuffer)
    {
        return framebuffer->getRenderPass();
    }
    else
    {
        return window->getOrCreateRenderPass();
    }
}

void RenderGraph::accept(RecordTraversal& recordTraversal) const
{
    if (window)
    {
        auto extent = window->extent2D();

        if (previous_extent.width == invalid_dimension || previous_extent.width == invalid_dimension || !windowResizeHandler)
        {
            previous_extent = extent;
        }
        else if (previous_extent.width != extent.width || previous_extent.height != extent.height)
        {
            auto this_renderGraph = const_cast<RenderGraph*>(this);

            auto& resizeHandler = *(this_renderGraph->windowResizeHandler);

            if (!resizeHandler.context) resizeHandler.context = vsg::Context::create(window->getDevice());

            resizeHandler.context->commandPool = recordTraversal.getState()->_commandBuffer->getCommandPool();
            resizeHandler.context->renderPass = window->getRenderPass();
            resizeHandler.renderArea = renderArea;
            resizeHandler.previous_extent = previous_extent;
            resizeHandler.new_extent = extent;
            resizeHandler.visited.clear();

            if (window->framebufferSamples() != VK_SAMPLE_COUNT_1_BIT) resizeHandler.context->overridePipelineStates.emplace_back(vsg::MultisampleState::create(window->framebufferSamples()));

            this_renderGraph->traverse(resizeHandler);

            this_renderGraph->renderArea = resizeHandler.renderArea;
            previous_extent = extent;
        }
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
    std::cout<<"RenaderGraph() "<<this<<" renderArea.offset.x = "<<renderArea.offset.x<<", renderArea.offset.y = "<<renderArea.offset.y
                <<", renderArea.extent.width = "<<renderArea.extent.width<<", renderArea.extent.height = "<<renderArea.extent.height<<std::endl;
#endif

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    VkCommandBuffer vk_commandBuffer = *(recordTraversal.getState()->_commandBuffer);
    vkCmdBeginRenderPass(vk_commandBuffer, &renderPassInfo, contents);

    // traverse the command buffer to place the commands into the command buffer.
    traverse(recordTraversal);

    vkCmdEndRenderPass(vk_commandBuffer);
}

ref_ptr<RenderGraph> vsg::createRenderGraphForView(ref_ptr<Window> window, ref_ptr<Camera> camera, ref_ptr<Node> scenegraph, VkSubpassContents contents)
{
    // set up the render graph for viewport & scene
    auto renderGraph = vsg::RenderGraph::create();

    renderGraph->addChild(View::create(ref_ptr<Camera>(camera), ref_ptr<Node>(scenegraph)));

    renderGraph->window = window;
    renderGraph->contents = contents;

    renderGraph->windowResizeHandler = WindowResizeHandler::create();

    if (camera)
    {
        renderGraph->renderArea = camera->getRenderArea();
    }
    else
    {
        renderGraph->renderArea.offset = {0, 0};
        renderGraph->renderArea.extent = window->extent2D();
    }

    if (window->framebufferSamples() != VK_SAMPLE_COUNT_1_BIT)
    {
        renderGraph->clearValues.resize(3);
        renderGraph->clearValues[0].color = window->clearColor();
        renderGraph->clearValues[1].color = window->clearColor();
        renderGraph->clearValues[2].depthStencil = VkClearDepthStencilValue{1.0f, 0};
    }
    else
    {
        renderGraph->clearValues.resize(2);
        renderGraph->clearValues[0].color = window->clearColor();
        renderGraph->clearValues[1].depthStencil = VkClearDepthStencilValue{1.0f, 0};
    }

    return renderGraph;
}
