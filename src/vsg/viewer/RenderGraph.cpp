/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/StateGroup.h>
#include <vsg/traversals/RecordTraversal.h>
#include <vsg/viewer/RenderGraph.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/State.h>

using namespace vsg;

namespace vsg
{
    class UpdatePipeline : public vsg::Visitor
    {
    public:
        Context context;

        UpdatePipeline(Device* device) :
            context(device) {}

        void apply(vsg::BindGraphicsPipeline& bindPipeline)
        {
            GraphicsPipeline* graphicsPipeline = bindPipeline.getPipeline();
            if (graphicsPipeline)
            {
                // TODO: Need to come up with a more general purpose way of matching Window viewports and GraphicsPipeline.
                bool containsViewport = false;
                bool containsContextWindowViewport = false;

                for (auto& pipelineState : graphicsPipeline->getPipelineStates())
                {
                    if (auto viewport = pipelineState.cast<ViewportState>())
                    {
                        containsViewport = true;
                        if (viewport == context.viewport) containsContextWindowViewport = true;
                    }
                }

                bool needToRegenerateGraphicsPipeline = containsContextWindowViewport || !containsViewport;
                if (needToRegenerateGraphicsPipeline)
                {
                    vsg::ref_ptr<vsg::GraphicsPipeline> new_pipeline = vsg::GraphicsPipeline::create(graphicsPipeline->getPipelineLayout(), graphicsPipeline->getShaderStages(), graphicsPipeline->getPipelineStates());

                    bindPipeline.release();

                    bindPipeline.setPipeline(new_pipeline);

                    bindPipeline.compile(context);
                }
            }
        }

        void apply(vsg::Object& object)
        {
            object.traverse(*this);
        }

        void apply(vsg::StateGroup& sg)
        {
            for (auto& command : sg.getStateCommands())
            {
                command->accept(*this);
            }
            sg.traverse(*this);
        }
    };
}; // namespace vsg

RenderGraph::RenderGraph()
{
}

void RenderGraph::accept(RecordTraversal& dispatchTraversal) const
{
    if (window)
    {
        auto extent = window->extent2D();

        if (previous_extent.width == invalid_dimension || previous_extent.width == invalid_dimension)
        {
            previous_extent = extent;
        }
        else if (previous_extent.width != extent.width || previous_extent.height != extent.height)
        {
            // crude handling of window resize...TODO, come up with a user controllable way to handle resize.

            vsg::UpdatePipeline updatePipeline(window->getDevice());

            updatePipeline.context.commandPool = dispatchTraversal.state->_commandBuffer->getCommandPool();
            updatePipeline.context.renderPass = window->getRenderPass();

            if (camera)
            {
                if (auto perspective = dynamic_cast<Perspective*>(camera->getProjectionMatrix()))
                {
                    perspective->aspectRatio = static_cast<double>(extent.width) / static_cast<double>(extent.height);
                }
                else if (auto ep = dynamic_cast<EllipsoidPerspective*>(camera->getProjectionMatrix()))
                {
                    ep->aspectRatio = static_cast<double>(extent.width) / static_cast<double>(extent.height);
                }

                auto viewport = camera->getViewportState();
                updatePipeline.context.viewport = viewport;

                viewport->getViewport().width = static_cast<float>(extent.width);
                viewport->getViewport().height = static_cast<float>(extent.height);
                viewport->getScissor().extent = extent;

                const_cast<RenderGraph*>(this)->renderArea.offset = VkOffset2D{0, 0}; // need to use offsets of viewport?
                const_cast<RenderGraph*>(this)->renderArea.extent = extent;
            }

            const_cast<RenderGraph*>(this)->traverse(updatePipeline);

            previous_extent = window->extent2D();
        }
    }

    if (camera)
    {
        dmat4 projMatrix, viewMatrix;
        camera->getProjectionMatrix()->get(projMatrix);
        camera->getViewMatrix()->get(viewMatrix);

        dispatchTraversal.setProjectionAndViewMatrix(projMatrix, viewMatrix);
    }

    VkCommandBuffer vk_commandBuffer = *(dispatchTraversal.state->_commandBuffer);

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    if (renderPass)
    {
        renderPassInfo.renderPass = *renderPass;
    }
    else if (window)
    {
        renderPassInfo.renderPass = *(window->getRenderPass());
    }
    if (framebuffer)
    {
        renderPassInfo.framebuffer = *framebuffer;
    }
    else if (window)
    {
        renderPassInfo.framebuffer = *(window->framebuffer(window->nextImageIndex()));
    }
    renderPassInfo.renderArea = renderArea;

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(vk_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // traverse the command buffer to place the commands into the command buffer.
    traverse(dispatchTraversal);

    vkCmdEndRenderPass(vk_commandBuffer);
}

ref_ptr<RenderGraph> vsg::createRenderGraphForView(Window* window, Camera* camera, Node* scenegraph)
{
    // set up the render graph for viewport & scene
    auto renderGraph = vsg::RenderGraph::create();
    renderGraph->addChild(ref_ptr<Node>(scenegraph));

    renderGraph->camera = camera;
    renderGraph->window = window;

    renderGraph->renderArea.offset = {0, 0};
    renderGraph->renderArea.extent = window->extent2D();

    renderGraph->clearValues.resize(2);
    renderGraph->clearValues[0].color = window->clearColor();
    renderGraph->clearValues[1].depthStencil = VkClearDepthStencilValue{1.0f, 0};

    return renderGraph;
}
