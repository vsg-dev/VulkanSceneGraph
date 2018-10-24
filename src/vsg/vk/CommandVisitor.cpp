/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/CommandVisitor.h>

#include <array>

#include <typeinfo>

namespace vsg
{


CommandVisitor::CommandVisitor(Framebuffer* framebuffer, RenderPass* renderPass, CommandBuffer* commandBuffer, const VkExtent2D& extent, const VkClearColorValue& clearColor) :
    _framebuffer(framebuffer),
    _renderPass(renderPass),
    _commandBuffer(commandBuffer),
    _extent(extent),
    _clearColor(clearColor)
{
}

void CommandVisitor::apply(Object& object)
{
    object.traverse(*this);
}

void CommandVisitor::apply(Node& object)
{
    object.traverse(*this);
}

void CommandVisitor::apply(Command& cmd)
{
    cmd.dispatch(*_commandBuffer);
}

void CommandVisitor::populateCommandBuffer(vsg::Node* subgraph)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    // if we are nested within a CommandBuffer already then use VkCommandBufferInheritanceInfo

    vkBeginCommandBuffer(*_commandBuffer, &beginInfo);

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = *_renderPass;
        renderPassInfo.framebuffer = *_framebuffer;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = _extent;

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = _clearColor;
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(*_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            subgraph->accept(*this);

        vkCmdEndRenderPass(*_commandBuffer);

    vkEndCommandBuffer(*_commandBuffer);
}

}
