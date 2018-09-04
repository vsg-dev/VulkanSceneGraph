/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/viewer/GraphicsStage.h>

#include <array>
#include <iostream>

namespace vsg
{

GraphicsVisitor::GraphicsVisitor(CommandBuffer* commandBuffer) :
    _commandBuffer(commandBuffer)
{
}

void GraphicsVisitor::apply(Node& node)
{
    std::cout<<"Visiting "<<typeid(node).name()<<" "<<this<<std::endl;
    node.traverse(*this);
}

void GraphicsVisitor::apply(StateGroup& stateGroup)
{

    std::cout<<"before GraphicsViitor::apply(StateGroup&)"<<std::endl;

    stateGroup.pushTo(_state);

    stateGroup.traverse(*this);

    stateGroup.popFrom(_state);

    std::cout<<"after GraphicsViitor::apply(StateGroup&)"<<std::endl;
}

void GraphicsVisitor::apply(Command& command)
{
    std::cout<<"before GraphicsViitor::apply(Comand& command)"<<typeid(command).name()<<std::endl;

    _state.dispatch(*_commandBuffer);
    command.dispatch(*_commandBuffer);
}

GraphicsStage::GraphicsStage(Node* commandGraph) :
    _commandGraph(commandGraph)
{
}

void GraphicsStage::populateCommandBuffer(CommandBuffer* commandBuffer, Framebuffer* framebuffer, RenderPass* renderPass, const VkExtent2D& extent2D, const VkClearColorValue& clearColor)
{
    vsg::GraphicsVisitor graphicsVisitor(commandBuffer);

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

        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(*commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            _commandGraph->accept(graphicsVisitor);

        vkCmdEndRenderPass(*commandBuffer);

    vkEndCommandBuffer(*commandBuffer);

}

}
