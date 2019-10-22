/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/StateGroup.h>

#include <vsg/vk/Extensions.h>
#include <vsg/viewer/RayTracingStage.h>

#include <vsg/traversals/CompileTraversal.h>

#include <array>
#include <limits>

using namespace vsg;

RayTracingStage::RayTracingStage(ref_ptr<Node> commandGraph, ref_ptr<RayTracingShaderBindings> shaderBindings, const VkExtent2D& extents, ref_ptr<Camera> camera) :
    _camera(camera),
    _commandGraph(commandGraph),
    _shaderBindings(shaderBindings),
    _projMatrix(new vsg::mat4Value),
    _viewMatrix(new vsg::mat4Value),
    _extent2D(extents)
{
}

void RayTracingStage::populateCommandBuffer(CommandBuffer* commandBuffer, Framebuffer* framebuffer, RenderPass* renderPass, const VkExtent2D& extent2D, const VkClearColorValue& clearColor)
{
    Extensions* extensions = Extensions::Get(_shaderBindings->device(), true);

    // if required get projection and view matrices from the Camera
    if (_camera)
    {
        if (_projMatrix) _camera->getProjectionMatrix()->get((*_projMatrix));
        if (_viewMatrix) _camera->getViewMatrix()->get((*_viewMatrix));
    }

    // set up the dispatching of the commands into the command buffer
    DispatchTraversal dispatchTraversal(commandBuffer, _maxSlot);
    dispatchTraversal.setProjectionAndViewMatrix(_projMatrix->value(), _viewMatrix->value());

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    // if we are nested within a CommandBuffer already then use VkCommandBufferInheritanceInfo

    vkBeginCommandBuffer(*commandBuffer, &beginInfo);

    // traverse the command buffer to place the commands into the command buffer.
    _commandGraph->accept(dispatchTraversal);

    extensions->vkCmdTraceRaysNV(*commandBuffer,
                     _shaderBindings->raygenTableBuffer(), _shaderBindings->raygeTableOffset(),
                     _shaderBindings->missTableBuffer(), _shaderBindings->missTableOffset(), _shaderBindings->missTableStride(),
                     _shaderBindings->hitTableBuffer(), _shaderBindings->hitTableOffset(), _shaderBindings->hitTableStride(),
                     _shaderBindings->callableTableBuffer(), _shaderBindings->callableTableOffset(), _shaderBindings->callableTableStride(),
                     _extent2D.width, _extent2D.height, 1);

    vkEndCommandBuffer(*commandBuffer);
}
