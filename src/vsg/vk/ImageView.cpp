/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Options.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/ImageView.h>

#include <iostream>

using namespace vsg;

ImageView::ImageView(Device* device, const VkImageViewCreateInfo& createInfo)
{
    std::cout<<"ImageView::ImageView(Device* device, const VkImageViewCreateInfo& createInfo) A"<<std::endl;

    _implementation[device->deviceID] = new Implementation(device, createInfo);
}

ImageView::ImageView(Device* device, Image* in_image, VkImageViewType type, VkFormat in_format, VkImageAspectFlags aspectFlags):
    image(in_image)
{
    std::cout<<"ImageView(Device* device, Image* in_image, VkImageViewType type, VkFormat in_format, VkImageAspectFlags aspectFlags): C"<<std::endl;

    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = *in_image;
    createInfo.viewType = type; // read from image?
    createInfo.format = in_format; // read from image?
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    createInfo.pNext = nullptr;

    _implementation[device->deviceID] = new Implementation(device, createInfo);
}

#if 0
void ImageView::compile(Context& context)
{
    if (_implementation[context.deviceID]) return;

    auto createInfo = context.scratchMemory->allocate<VkImageViewCreateInfo>();
    createInfo->sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo->pNext = nullptr;

    createInfo->flags = flags;
    createInfo->image = *image; // should be image-vk(context.deviceID);
    createInfo->viewType = viewType;
    createInfo->format = format;
    createInfo->components = components;
    createInfo->subresourceRange = subresourceRange;

    _implementation[context.deviceID] = new Implementation(context.device, *createInfo);
}
#endif

ImageView::~ImageView()
{
}


ImageView::Implementation::Implementation(Device* in_device, const VkImageViewCreateInfo& createInfo) :
    device(in_device)
{
    if (VkResult result = vkCreateImageView(*device, &createInfo, device->getAllocationCallbacks(), &imageView); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create ImageView.", result};
    }
}

ImageView::Implementation::~Implementation()
{
}

ref_ptr<ImageView> vsg::createImageView(vsg::Context& context, const VkImageCreateInfo& imageCreateInfo, VkImageAspectFlags aspectFlags)
{
    std::cout<<"vsg::createImageView(vsg::Context& context, const VkImageCreateInfo& imageCreateInfo, VkImageAspectFlags aspectFlags) D"<<std::endl;

    vsg::Device* device = context.device;

    vsg::ref_ptr<vsg::Image> image;

    image = vsg::Image::create(device, imageCreateInfo);

    // get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(*device, *image, &memRequirements);

    // allocate memory with out export memory info extension
    auto [deviceMemory, offset] = context.deviceMemoryBufferPools->reserveMemory(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (!deviceMemory)
    {
        throw Exception{"Error: Failed allocate memory for image.", 0};
    }

    image->bind(deviceMemory, offset);

    return vsg::ImageView::create(device, image, VK_IMAGE_VIEW_TYPE_2D, imageCreateInfo.format, aspectFlags);
}

ref_ptr<ImageView> vsg::createImageView(Device* device, const VkImageCreateInfo& imageCreateInfo, VkImageAspectFlags aspectFlags)
{
    std::cout<<"vsg::createImageView(Device* device, const VkImageCreateInfo& imageCreateInfo, VkImageAspectFlags aspectFlags) E"<<std::endl;

    vsg::ref_ptr<vsg::Image> image;

    image = vsg::Image::create(device, imageCreateInfo);

    // allocate memory with out export memory info extension
    auto deviceMemory = DeviceMemory::create(device, image->getMemoryRequirements(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (!deviceMemory)
    {
        throw Exception{"Error: Failed allocate memory for image.", 0};
    }

    image->bind(deviceMemory, 0);

    return vsg::ImageView::create(device, image, VK_IMAGE_VIEW_TYPE_2D, imageCreateInfo.format, aspectFlags);
}
