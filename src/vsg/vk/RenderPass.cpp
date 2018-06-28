#include <vsg/vk/RenderPass.h>

#include <iostream>

namespace vsg
{

RenderPass::RenderPass(Device* device, VkRenderPass renderPass, VkAllocationCallbacks* pAllocator) :
    _device(device),
    _renderPass(renderPass),
    _pAllocator(pAllocator)
{
}

RenderPass::RenderPass(Device* device, VkFormat imageFormat, VkAllocationCallbacks* pAllocator) :
    _device(device),
    _renderPass(VK_NULL_HANDLE),
    _pAllocator(pAllocator)
{
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

    if (vkCreateRenderPass(*device, &renderPassInfo, pAllocator, &_renderPass) != VK_SUCCESS)
    {
        std::cout<<"Failed to create VkRenderPass."<<std::endl;
    }
}

RenderPass::~RenderPass()
{
    if (_renderPass)
    {
        std::cout<<"Calling vkDestroyRenderPass"<<std::endl;
        vkDestroyRenderPass(*_device, _renderPass, _pAllocator);
    }
}

}