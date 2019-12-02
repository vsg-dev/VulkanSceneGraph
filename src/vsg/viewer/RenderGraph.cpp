/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/viewer/RenderGraph.h>
#include <vsg/traversals/DispatchTraversal.h>
#include <vsg/vk/State.h>

using namespace vsg;

RenderGraph::RenderGraph()
{
}

void RenderGraph::accept(DispatchTraversal& dispatchTraversal) const
{
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
    renderPassInfo.renderPass = *(window->renderPass());
    renderPassInfo.framebuffer = *(window->framebuffer(window->nextImageIndex()));
    renderPassInfo.renderArea = renderArea;

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(vk_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // traverse the command buffer to place the commands into the command buffer.
    traverse(dispatchTraversal);

    vkCmdEndRenderPass(vk_commandBuffer);
}


