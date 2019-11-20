/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/StateGroup.h>

#include <vsg/vk/Extensions.h>
#include <vsg/vk/PipelineBarrier.h>

#include <vsg/raytracing/RayTracingStage.h>

#include <vsg/ui/ApplicationEvent.h>

#include <vsg/traversals/CompileTraversal.h>

#include <array>
#include <limits>

using namespace vsg;

RayTracingStage::RayTracingStage(ref_ptr<Node> commandGraph, ref_ptr<RayTracingShaderBindings> shaderBindings, ImageView* storageImage, const VkExtent2D& extents, ref_ptr<Camera> camera) :
    _camera(camera),
    _commandGraph(commandGraph),
    _shaderBindings(shaderBindings),
    _projMatrix(new vsg::mat4Value),
    _viewMatrix(new vsg::mat4Value),
    _storageImage(storageImage),
    _extent2D(extents)
{
}

void RayTracingStage::populateCommandBuffer(CommandBuffer* commandBuffer, Framebuffer* framebuffer, RenderPass* renderPass, ImageView* imageView, const VkExtent2D& extent, const VkClearColorValue& clearColor, ref_ptr<FrameStamp> frameStamp)
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


    //  transition image layouts for copy
    auto imb_transitionSwapChainToWriteDest = ImageMemoryBarrier::create(
        0, VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
        ref_ptr<Image>(imageView->getImage()),
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    auto pb_transitionSwapChainToWriteDest = PipelineBarrier::create(
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0, imb_transitionSwapChainToWriteDest);

    pb_transitionSwapChainToWriteDest->dispatch(*commandBuffer);


    auto ibm_transitionStorageImageToReadSrc = vsg::ImageMemoryBarrier::create(
        0, VK_ACCESS_TRANSFER_READ_BIT,
        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
        ref_ptr<Image>(_storageImage->getImage()),
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    auto pb_transitionStorageImageToReadSrc = PipelineBarrier::create(
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0, ibm_transitionStorageImageToReadSrc);

    pb_transitionStorageImageToReadSrc->dispatch(*commandBuffer);


    // copy storage image to swap chain
    VkImageCopy copyRegion{};
    copyRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copyRegion.dstOffset = {0, 0, 0};
    copyRegion.extent = {_extent2D.width, _extent2D.height, 1};
    vkCmdCopyImage(*commandBuffer, *_storageImage->getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *imageView->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);


    // transition image layouts back
    auto imb_transitionSwapChainToOriginal = ImageMemoryBarrier::create(
        VK_ACCESS_TRANSFER_WRITE_BIT, 0,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
        ref_ptr<Image>(imageView->getImage()),
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    auto pb_transitionSwapChainToOriginal = PipelineBarrier::create(
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0, imb_transitionSwapChainToOriginal);

    pb_transitionSwapChainToOriginal->dispatch(*commandBuffer);

    auto imb_transitionStorageImageToOriginal = ImageMemoryBarrier::create(
        VK_ACCESS_TRANSFER_READ_BIT, 0,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
        VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
        ref_ptr<Image>(_storageImage->getImage()),
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    auto pb_transitionStorageImageToOriginal = PipelineBarrier::create(
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0, imb_transitionStorageImageToOriginal);

    pb_transitionStorageImageToOriginal->dispatch(*commandBuffer);


    vkEndCommandBuffer(*commandBuffer);
}
