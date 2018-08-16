#include <vsg/vk/ImageView.h>

namespace vsg
{

ImageView::ImageView(VkImageView imageView, Device* device, Image* image, AllocationCallbacks* allocator) :
    _imageView(imageView),
    _device(device),
    _image(image),
    _allocator(allocator)
{
}

ImageView::~ImageView()
{
    if (_imageView)
    {
        vkDestroyImageView(*_device, _imageView, _allocator);
    }
}

ImageView::Result ImageView::create(Device* device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, AllocationCallbacks* allocator)
{
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView view;
    VkResult result = vkCreateImageView(*device, &createInfo, allocator, &view);
    if (result==VK_SUCCESS)
    {
        return new ImageView(view, device, nullptr, allocator);
    }
    else
    {
        return Result("Error: Failed to create ImageView.", result);
    }
}

ImageView::Result ImageView::create(Device* device, Image* image, VkFormat format, VkImageAspectFlags aspectFlags, AllocationCallbacks* allocator)
{
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = *image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // read from image?
    createInfo.format = format;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView view;
    VkResult result = vkCreateImageView(*device, &createInfo, allocator, &view);
    if (result==VK_SUCCESS)
    {
        return new ImageView(view, device, image, allocator);
    }
    else
    {
        return Result("Error: Failed to create ImageView.", result);
    }
}

}
