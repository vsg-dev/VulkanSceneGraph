#include <vsg/vk/Image.h>

#include <iostream>

namespace vsg
{

Image::Image(Device* device, VkImage Image, AllocationCallbacks* allocator) :
    _device(device),
    _image(Image),
    _allocator(allocator)
{
}

Image::~Image()
{
    if (_image)
    {
        std::cout<<"Calling vkDestroyImage"<<std::endl;
        vkDestroyImage(*_device, _image, *_allocator);
    }
}

Image::Result Image::create(Device* device, const VkImageCreateInfo& createImageInfo, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Image::Result("Error: vsg::Image::create(...) failed to create vkImage, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkImage image;
    VkResult result = vkCreateImage(*device, &createImageInfo, *allocator, &image);
    if (result == VK_SUCCESS)
    {
        return new Image(device, image, allocator);
    }
    else
    {
        return Result("Error: Failed to create vkImage.", result);
    }
}

}