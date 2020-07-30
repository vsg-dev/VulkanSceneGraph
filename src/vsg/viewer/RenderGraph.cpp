/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
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
                struct ContainsViewport : public ConstVisitor
                {
                    bool foundViewport = false;
                    void apply(const ViewportState&) override { foundViewport = true; }
                    bool operator()(const GraphicsPipeline& gp)
                    {
                        for (auto& pipelineState : gp.getPipelineStates())
                        {
                            pipelineState->accept(*this);
                        }
                        return foundViewport;
                    }
                } containsViewport;

                bool needToRegenerateGraphicsPipeline = !containsViewport(*graphicsPipeline);
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

        if (previous_extent.width == invalid_dimension || previous_extent.width == invalid_dimension)
        {
            previous_extent = extent;
        }
        else if (previous_extent.width != extent.width || previous_extent.height != extent.height)
        {
            // crude handling of window resize...TODO, come up with a user controllable way to handle resize.

            vsg::UpdatePipeline updatePipeline(window->getDevice());

            updatePipeline.context.commandPool = recordTraversal.getState()->_commandBuffer->getCommandPool();
            updatePipeline.context.renderPass = window->getRenderPass();

            if (camera)
            {
                camera->getProjectionMatrix()->changeExtent(previous_extent, extent);

                auto viewportState = camera->getViewportState();
                viewportState->set(extent);
                updatePipeline.context.defaultPipelineStates.emplace_back(viewportState);

                const_cast<RenderGraph*>(this)->renderArea.offset = VkOffset2D{0, 0}; // need to use offsets of viewport?
                const_cast<RenderGraph*>(this)->renderArea.extent = extent;
            }

            if (window->framebufferSamples() != VK_SAMPLE_COUNT_1_BIT) updatePipeline.context.overridePipelineStates.emplace_back(vsg::MultisampleState::create(window->framebufferSamples()));

            const_cast<RenderGraph*>(this)->traverse(updatePipeline);

            previous_extent = window->extent2D();
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

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    VkCommandBuffer vk_commandBuffer = *(recordTraversal.getState()->_commandBuffer);
    vkCmdBeginRenderPass(vk_commandBuffer, &renderPassInfo, contents);

    if (camera)
    {
        dmat4 projMatrix, viewMatrix;
        camera->getProjectionMatrix()->get(projMatrix);
        camera->getViewMatrix()->get(viewMatrix);

        recordTraversal.setProjectionAndViewMatrix(projMatrix, viewMatrix);
    }

    // traverse the command buffer to place the commands into the command buffer.
    traverse(recordTraversal);

    vkCmdEndRenderPass(vk_commandBuffer);
}

ref_ptr<RenderGraph> vsg::createRenderGraphForView(Window* window, Camera* camera, Node* scenegraph, VkSubpassContents contents)
{
    // set up the render graph for viewport & scene
    auto renderGraph = vsg::RenderGraph::create();
    renderGraph->addChild(ref_ptr<Node>(scenegraph));

    renderGraph->camera = camera;
    renderGraph->window = window;
    renderGraph->contents = contents;

    renderGraph->renderArea.offset = {0, 0};
    renderGraph->renderArea.extent = window->extent2D();

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
