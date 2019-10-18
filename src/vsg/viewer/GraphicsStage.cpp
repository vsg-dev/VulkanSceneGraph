/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/StateGroup.h>

#include <vsg/viewer/GraphicsStage.h>

#include <vsg/traversals/CompileTraversal.h>

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
    _projMatrix(new vsg::mat4Value),
    _viewMatrix(new vsg::mat4Value),
    _extent2D{std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()}
{
}

void GraphicsStage::populateCommandBuffer(CommandBuffer* commandBuffer, Framebuffer* framebuffer, RenderPass* renderPass, const VkExtent2D& extent2D, const VkClearColorValue& clearColor)
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
    DispatchTraversal dispatchTraversal(commandBuffer, _maxSlot);
    dispatchTraversal.setProjectionAndViewMatrix(_projMatrix->value(), _viewMatrix->value());

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    // if we are nested within a CommandBuffer already then use VkCommandBufferInheritanceInfo

    //vkBeginCommandBuffer(*commandBuffer, &beginInfo);

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

    //vkEndCommandBuffer(*commandBuffer);
}

//
// OffscreenGraphicsStage
//

OffscreenGraphicsStage::OffscreenGraphicsStage(Device* device, ref_ptr<Node> commandGraph, ref_ptr<Camera> camera, VkExtent2D extents, ref_ptr<ImageView> colorImageView, ref_ptr<ImageView> depthImageView, VkAttachmentLoadOp loadOp) :
    Inherit(commandGraph, camera)
{
    _extent2D = extents;

    VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageLayout colorFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
    VkImageLayout depthFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //
    // create render pass
    //

    RenderPass::Attachments attachments;

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = colorFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = loadOp;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // for now make this store
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = colorFinalLayout;
    attachments.push_back(colorAttachment);

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = loadOp;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;  // for now make this store
    depthAttachment.stencilLoadOp = loadOp;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = depthFinalLayout;
    attachments.push_back(depthAttachment);

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RenderPass::Subpasses subpasses;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpasses.push_back(subpass);

    RenderPass::Dependancies dependancies;
    VkSubpassDependency dependency = {};

    
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependancies.push_back(dependency);

    dependency.srcSubpass = 0;
    dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependancies.push_back(dependency);


    _renderPass = RenderPass::create(device, attachments, subpasses, dependancies);

    //
    // create color and depth images for frame buffer attachments
    //

    _colorImageView = colorImageView;
    _depthImageView = depthImageView;

    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = _extent2D.width;
    imageCreateInfo.extent.height = _extent2D.height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.pNext = nullptr;

    if (!_colorImageView)
    {
        // create color
        imageCreateInfo.format = colorFormat;
        imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT; // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        _colorImage = Image::create(device, imageCreateInfo);
        _colorImageMemory = DeviceMemory::create(device, _colorImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkBindImageMemory(*device, *_colorImage, *_colorImageMemory, 0);

        _colorImageView = ImageView::create(device, _colorImage, VK_IMAGE_VIEW_TYPE_2D, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    if (!_depthImageView)
    {
        // create depth
        imageCreateInfo.format = depthFormat;
        imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        _depthImage = Image::create(device, imageCreateInfo);
        _depthImageMemory = DeviceMemory::create(device, _depthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkBindImageMemory(*device, *_depthImage, *_depthImageMemory, 0);

        _depthImageView = ImageView::create(device, _depthImage, VK_IMAGE_VIEW_TYPE_2D, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    }

    std::array<VkImageView, 2> fbAttachments = {{*_colorImageView, *_depthImageView}};

    //
    // create frame buffer
    //

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = *_renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
    framebufferInfo.pAttachments = fbAttachments.data();
    framebufferInfo.width = _extent2D.width;
    framebufferInfo.height = _extent2D.height;
    framebufferInfo.layers = 1;

    _frameBuffer = Framebuffer::create(device, framebufferInfo);
    _semaphore = vsg::Semaphore::create(device);
}

void OffscreenGraphicsStage::populateCommandBuffer(CommandBuffer* commandBuffer, Framebuffer* framebuffer, RenderPass* renderPass, const VkExtent2D& extent2D, const VkClearColorValue& clearColor)
{
    if (!_viewport)
    {
        if (_camera)
        {
            ref_ptr<Perspective> perspective(dynamic_cast<Perspective*>(_camera->getProjectionMatrix()));
            if (perspective)
            {
                perspective->aspectRatio = static_cast<double>(_extent2D.width) / static_cast<double>(_extent2D.height);
            }

            if (_camera->getViewportState())
            {
                //_camera->getViewportState()->getViewport().width = static_cast<float>(_extent2D.width);
                //_camera->getViewportState()->getViewport().height = static_cast<float>(_extent2D.height);
                _camera->getViewportState()->getScissor().extent = _extent2D;
            }
            else
            {
                _camera->setViewportState(ViewportState::create(_extent2D));
            }
            _viewport = _camera->getViewportState();
        }
        else
        {
            _viewport = ViewportState::create(_extent2D);
        }
    }

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

    //vkBeginCommandBuffer(*commandBuffer, &beginInfo);

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = *_renderPass;
    renderPassInfo.framebuffer = *_frameBuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _extent2D;

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {0.2f, 0.9f, 0.2f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    //renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    //renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(*commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // traverse the command buffer to place the commands into the command buffer.
    _commandGraph->accept(dispatchTraversal);

    vkCmdEndRenderPass(*commandBuffer);

    //vkEndCommandBuffer(*commandBuffer);
}