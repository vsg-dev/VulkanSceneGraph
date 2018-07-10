#include <vsg/vk/RenderPass.h>

#include <iostream>

namespace vsg
{

RenderPass::RenderPass(Device* device, VkRenderPass renderPass, AllocationCallbacks* allocator) :
    _device(device),
    _renderPass(renderPass),
    _allocator(allocator)
{
}

RenderPass::Result RenderPass::create(Device* device, VkFormat imageFormat, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Result("Error: vsg::RenderPass::create(...) failed to create RenderPass, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkRenderPass renderPass;
    VkResult result = vkCreateRenderPass(*device, &renderPassInfo, *allocator, &renderPass);

    if (result == VK_SUCCESS)
    {
        return new RenderPass(device, renderPass, allocator);
    }
    else
    {
        return Result("Error: vsg::RenderPass::create(...) Failed to create VkRenderPass.", result);
    }
}

RenderPass::~RenderPass()
{
    if (_renderPass)
    {
        std::cout<<"Calling vkDestroyRenderPass"<<std::endl;
        vkDestroyRenderPass(*_device, _renderPass, *_allocator);
    }
}

}