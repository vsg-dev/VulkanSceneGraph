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
#include <vsg/viewer/View.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/State.h>

#include <iostream>

using namespace vsg;

#define NEW_CODE 1

namespace vsg
{
    template<typename T, typename R>
    T scale_parameter(T original, R extentOriginal, R extentNew)
    {
        if (original == static_cast<T>(extentOriginal)) return static_cast<T>(extentNew);
        return static_cast<T>(static_cast<float>(original) * static_cast<float>(extentNew) / static_cast<float>(extentOriginal)+0.5f);
    }

    class WindowResizeHandler : public vsg::Visitor
    {
    public:
        Context context;
        VkRect2D renderArea;
        VkExtent2D previous_extent;
        VkExtent2D new_extent;
        std::set<Object*> visited;

        WindowResizeHandler(Device* device) :
            context(device) {}


        bool visit(Object* object)
        {
            if (visited.count(object) != 0) return false;
            visited.insert(object);
            return true;
        }

        void apply(vsg::BindGraphicsPipeline& bindPipeline)
        {
            GraphicsPipeline* graphicsPipeline = bindPipeline.pipeline;

            if (!visit(graphicsPipeline)) return;

            if (graphicsPipeline)
            {
                struct ContainsViewport : public ConstVisitor
                {
                    bool foundViewport = false;
                    void apply(const ViewportState&) override { foundViewport = true; }
                    bool operator()(const GraphicsPipeline& gp)
                    {
                        for (auto& pipelineState : gp.pipelineStates)
                        {
                            pipelineState->accept(*this);
                        }
                        return foundViewport;
                    }
                } containsViewport;

                bool needToRegenerateGraphicsPipeline = !containsViewport(*graphicsPipeline);
                if (needToRegenerateGraphicsPipeline)
                {
                    vsg::ref_ptr<vsg::GraphicsPipeline> new_pipeline = vsg::GraphicsPipeline::create(graphicsPipeline->layout, graphicsPipeline->stages, graphicsPipeline->pipelineStates);

                    bindPipeline.release();

                    bindPipeline.pipeline = new_pipeline;

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
            if (!visit(&sg)) return;

            for (auto& command : sg.getStateCommands())
            {
                command->accept(*this);
            }
            sg.traverse(*this);
        }

        void apply(vsg::View& view)
        {
            if (!visit(&view)) return;

            if (!view.camera)
            {
                view.traverse(*this);
                return;
            }

            view.camera->getProjectionMatrix()->changeExtent(previous_extent, new_extent);

            auto viewportState = view.camera->getViewportState();

            size_t num_viewports = std::min(viewportState->viewports.size(), viewportState->scissors.size());
            for(size_t i = 0; i<num_viewports; ++i)
            {
                auto& viewport = viewportState->viewports[i];
                auto& scissor = viewportState->scissors[i];

                bool renderAreaMatches = (renderArea.offset.x == scissor.offset.x) && (renderArea.offset.y == scissor.offset.y) &&
                                         (renderArea.extent.width == scissor.extent.width) && (renderArea.extent.height == scissor.extent.height);

                int32_t edge_x = scissor.offset.x + static_cast<int32_t>(scissor.extent.width);
                int32_t edge_y = scissor.offset.y + static_cast<int32_t>(scissor.extent.height);

                scissor.offset.x = scale_parameter(scissor.offset.x, previous_extent.width, new_extent.width);
                scissor.offset.y = scale_parameter(scissor.offset.y, previous_extent.height, new_extent.height);
                scissor.extent.width = static_cast<uint32_t>(scale_parameter(edge_x, previous_extent.width, new_extent.width) - scissor.offset.x);
                scissor.extent.height = static_cast<uint32_t>(scale_parameter(edge_y, previous_extent.height, new_extent.height) - scissor.offset.y);

                viewport.x = static_cast<float>(scissor.offset.x);
                viewport.y = static_cast<float>(scissor.offset.y);
                viewport.width = static_cast<float>(scissor.extent.width);
                viewport.height = static_cast<float>(scissor.extent.height);

                if (renderAreaMatches)
                {
                    renderArea = scissor;
                }
            }

            context.defaultPipelineStates.emplace_back(viewportState);

            view.traverse(*this);

            context.defaultPipelineStates.pop_back();
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

            vsg::WindowResizeHandler resizeHandler(window->getDevice());

            resizeHandler.context.commandPool = recordTraversal.getState()->_commandBuffer->getCommandPool();
            resizeHandler.context.renderPass = window->getRenderPass();
            resizeHandler.renderArea = renderArea;
            resizeHandler.previous_extent = previous_extent;
            resizeHandler.new_extent = extent;

            if (window->framebufferSamples() != VK_SAMPLE_COUNT_1_BIT) resizeHandler.context.overridePipelineStates.emplace_back(vsg::MultisampleState::create(window->framebufferSamples()));

            const_cast<RenderGraph*>(this)->traverse(resizeHandler);

            previous_extent = extent;
            const_cast<RenderGraph*>(this)->renderArea = resizeHandler.renderArea;

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

ref_ptr<RenderGraph> vsg::createRenderGraphForView(Window* window, Camera* camera, Node* scenegraph, VkSubpassContents contents)
{
    // set up the render graph for viewport & scene
    auto renderGraph = vsg::RenderGraph::create();

    renderGraph->addChild(View::create(ref_ptr<Camera>(camera), ref_ptr<Node>(scenegraph)));

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
