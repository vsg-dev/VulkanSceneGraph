#include <vsg/vk/Framebuffer.h>

#include <iostream>

namespace vsg
{

Framebuffer::Framebuffer(Device* device, VkFramebuffer framebuffer, AllocationCallbacks* allocator) :
    _device(device),
    _framebuffer(framebuffer),
    _allocator(allocator)
{
}

Framebuffer::~Framebuffer()
{
    if (_framebuffer)
    {
        std::cout<<"Calling vkDestroyFramebuffer"<<std::endl;
        vkDestroyFramebuffer(*_device, _framebuffer, *_allocator);
    }
}

Framebuffers createFrameBuffers(Device* device, Swapchain* swapchain, RenderPass* renderPass, AllocationCallbacks* allocator)
{
    const Swapchain::ImageViews& imageViews = swapchain->getImageViews();
    const VkExtent2D& extent = swapchain->getExtent();

    Framebuffers framebuffers;
    for(auto imageView : imageViews)
    {
        VkImageView attachments[] =
        {
            *imageView
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        VkFramebuffer framebuffer;
        if (vkCreateFramebuffer(*device,&framebufferInfo, *allocator, &framebuffer) == VK_SUCCESS)
        {
            framebuffers.push_back(new Framebuffer(device, framebuffer, allocator));
        }
        else
        {
            std::cout<<"Failing to create framebuffer for "<<&imageView<<std::endl;
        }
    }
    return framebuffers;
}

}