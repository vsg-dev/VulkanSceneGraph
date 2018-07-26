#include <vsg/vk/ImageView.h>

#include <iostream>

namespace vsg
{

ImageView::ImageView(Device* device, VkImageView imageView, AllocationCallbacks* allocator) :
    _device(device),
    _imageView(imageView),
    _allocator(allocator)
{
}

ImageView::~ImageView()
{
    if (_imageView)
    {
        std::cout<<"Calling vkDestroyImageView(..)"<<std::endl;
        vkDestroyImageView(*_device, _imageView, *_allocator);
    }
}

ImageView::Result ImageView::create(Device* device, VkImage image, VkFormat format, AllocationCallbacks* allocator)
{
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView view;
    VkResult result = vkCreateImageView(*device, &createInfo, *allocator, &view);
    if (result==VK_SUCCESS)
    {
        return new ImageView(device, view, allocator);
    }
    else
    {
        return Result("Error: Failed to create ImageView.", result);
    }
}


}
