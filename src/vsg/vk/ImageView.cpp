/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/ImageView.h>

using namespace vsg;

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

ImageView::Result ImageView::create(Device* device, const VkImageViewCreateInfo& createInfo, AllocationCallbacks* allocator)
{
    VkImageView view;
    VkResult result = vkCreateImageView(*device, &createInfo, allocator, &view);
    if (result == VK_SUCCESS)
    {
        return Result(new ImageView(view, device, nullptr, allocator));
    }
    else
    {
        return Result("Error: Failed to create ImageView.", result);
    }
}

ImageView::Result ImageView::create(Device* device, VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspectFlags, AllocationCallbacks* allocator)
{
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = type;
    createInfo.format = format;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    createInfo.pNext = nullptr;

    return create(device, createInfo, allocator);
}

ImageView::Result ImageView::create(Device* device, Image* image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspectFlags, AllocationCallbacks* allocator)
{
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = *image;
    createInfo.viewType = type; // read from image?
    createInfo.format = format; // read from image?
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    createInfo.pNext = nullptr;

    Result result = create(device, createInfo, allocator);
    if (result) result.object()->setImage(image);
    return result;
}
