#include <vsg/vk/Image.h>

#include <iostream>

namespace vsg
{

Image::Image(VkImage image, Device* device, AllocationCallbacks* allocator) :
    _image(image),
    _device(device),
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
        return new Image(image, device, allocator);
    }
    else
    {
        return Result("Error: Failed to create vkImage.", result);
    }
}


ImageMemoryBarrier::ImageMemoryBarrier(VkAccessFlags in_srcAccessMask, VkAccessFlags in_destAccessMask,
                    VkImageLayout in_oldLayout, VkImageLayout in_newLayout,
                    Image* in_image) :
                    VkImageMemoryBarrier{},
                    _image(in_image)
{
    sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    oldLayout = in_oldLayout;
    newLayout = in_newLayout;
    srcAccessMask = in_srcAccessMask;
    dstAccessMask = in_destAccessMask;
    srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image = *in_image;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;
}

ImageMemoryBarrier::~ImageMemoryBarrier()
{
}

void ImageMemoryBarrier::cmdPiplineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage)
{
    std::cout<<"vkCmdPipelineBarrier("<<std::endl;
    std::cout<<"    sourceStage = "<<sourceStage<<std::endl;
    std::cout<<"    destinationStage="<<destinationStage<<std::endl;
    std::cout<<"    srcAccessMask = 0x"<<std::hex<<srcAccessMask<<std::endl;
    std::cout<<"    dstAccessMask = 0x"<<dstAccessMask<<std::endl;
    std::cout<<"    oldLayout = "<<std::dec<<oldLayout<<std::endl;
    std::cout<<"    newLayout = "<<newLayout<<std::endl;

    vkCmdPipelineBarrier(commandBuffer,
                        sourceStage, destinationStage,
                        0,
                        0, nullptr,
                        0, nullptr,
                        1, static_cast<VkImageMemoryBarrier*>(this));
}

}
