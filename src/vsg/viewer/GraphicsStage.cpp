/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/StateGroup.h>

#include <vsg/viewer/GraphicsStage.h>

#include <vsg/traversals/CompileTraversal.h>

#include <vsg/ui/ApplicationEvent.h>

#include <array>
#include <limits>

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
                bool needToRegenerateGraphicsPipeline = false;
                for (auto& pipelineState : graphicsPipeline->getPipelineStates())
                {
                    if (pipelineState == context.viewport)
                    {
                        needToRegenerateGraphicsPipeline = true;
                        break;
                    }
                }

                if (graphicsPipeline->getImplementation())
                {
                    for (auto& pipelineState : graphicsPipeline->getImplementation()->_pipelineStates)
                    {
                        if (pipelineState == context.viewport)
                        {
                            needToRegenerateGraphicsPipeline = true;
                            break;
                        }
                    }
                }

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
#if 1
                BindGraphicsPipeline* bindGaphicsPipeline = dynamic_cast<BindGraphicsPipeline*>(command.get());
                if (bindGaphicsPipeline)
                {
                    apply(*bindGaphicsPipeline);
                }
#else
                command->accept(*this);
#endif
            }
            sg.traverse(*this);
        }
    };
}; // namespace vsg

GraphicsStage::GraphicsStage(ref_ptr<Node> commandGraph, ref_ptr<Camera> camera) :
    _camera(camera),
    _commandGraph(commandGraph),
    _projMatrix(new vsg::dmat4Value),
    _viewMatrix(new vsg::dmat4Value),
    _extent2D{std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()}
{
}

void GraphicsStage::populateCommandBuffer(CommandBuffer* commandBuffer, Framebuffer* framebuffer, RenderPass* renderPass, ImageView* /*imageView*/, const VkExtent2D& extent2D, const VkClearColorValue& clearColor, ref_ptr<FrameStamp> frameStamp)
{
    // handle any changes in window size
    if (_extent2D.width == std::numeric_limits<uint32_t>::max())
    {
        _extent2D = extent2D;

        if (_camera)
        {
            ref_ptr<Perspective> perspective(dynamic_cast<Perspective*>(_camera->getProjectionMatrix()));
            if (perspective)
            {
                perspective->aspectRatio = static_cast<double>(extent2D.width) / static_cast<double>(extent2D.height);
            }

            if (_camera->getViewportState())
            {
                _camera->getViewportState()->getViewport().width = static_cast<float>(extent2D.width);
                _camera->getViewportState()->getViewport().height = static_cast<float>(extent2D.height);
                _camera->getViewportState()->getScissor().extent = extent2D;
            }
            else
            {
                _camera->setViewportState(ViewportState::create(extent2D));
            }
            _viewport = _camera->getViewportState();
        }
        else if (!_viewport)
        {
            _viewport = ViewportState::create(extent2D);
        }
    }
    else if ((_extent2D.width != extent2D.width) || (_extent2D.height != extent2D.height))
    {
        _extent2D = extent2D;

        if (_camera)
        {
            ref_ptr<Perspective> perspective(dynamic_cast<Perspective*>(_camera->getProjectionMatrix()));
            if (perspective)
            {
                perspective->aspectRatio = static_cast<double>(extent2D.width) / static_cast<double>(extent2D.height);
            }
        }

        _viewport->getViewport().width = static_cast<float>(extent2D.width);
        _viewport->getViewport().height = static_cast<float>(extent2D.height);
        _viewport->getScissor().extent = extent2D;

        vsg::UpdatePipeline updatePipeline(commandBuffer->getDevice());

        updatePipeline.context.commandPool = commandBuffer->getCommandPool();
        updatePipeline.context.renderPass = renderPass;
        updatePipeline.context.viewport = _viewport;

        _commandGraph->accept(updatePipeline);
    }

    // if required get projection and view matrices from the Camera
    if (_camera)
    {
        if (_projMatrix) _camera->getProjectionMatrix()->get((*_projMatrix));
        if (_viewMatrix) _camera->getViewMatrix()->get((*_viewMatrix));
    }

    // set up the dispatching of the commands into the command buffer
    DispatchTraversal dispatchTraversal(commandBuffer, _maxSlot, frameStamp);
    dispatchTraversal.setProjectionAndViewMatrix(_projMatrix->value(), _viewMatrix->value());

    dispatchTraversal.databasePager = databasePager;
    if (databasePager) dispatchTraversal.culledPagedLODs = databasePager->culledPagedLODs;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    // if we are nested within a CommandBuffer already then use VkCommandBufferInheritanceInfo

    vkBeginCommandBuffer(*commandBuffer, &beginInfo);

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = *renderPass;
    renderPassInfo.framebuffer = *framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent2D;

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = clearColor;
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(*commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // traverse the command buffer to place the commands into the command buffer.
    _commandGraph->accept(dispatchTraversal);

    vkCmdEndRenderPass(*commandBuffer);

    vkEndCommandBuffer(*commandBuffer);
}
