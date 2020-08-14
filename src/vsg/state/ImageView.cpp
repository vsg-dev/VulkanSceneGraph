/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Options.h>
#include <vsg/state/ImageView.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/Context.h>

using namespace vsg;

void ImageView::VulkanData::release()
{
    if (imageView)
    {
        vkDestroyImageView(*device, imageView, device->getAllocationCallbacks());
        imageView = VK_NULL_HANDLE;
        device = {};
    }
}

ImageView::CreateInfo::CreateInfo(ref_ptr<Image> in_image, VkImageAspectFlags aspectFlags) :
    image(in_image)
{
    if (image && image->createInfo)
    {
        auto imageCreateInfo = image->createInfo.get();
        auto imageType = imageCreateInfo->imageType;

        viewType = (imageType==VK_IMAGE_TYPE_3D) ? VK_IMAGE_VIEW_TYPE_3D : ((imageType==VK_IMAGE_TYPE_2D) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_1D);
        format = imageCreateInfo->format;
        subresourceRange.aspectMask = aspectFlags;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = imageCreateInfo->mipLevels;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = imageCreateInfo->arrayLayers;
    }
}

void ImageView::CreateInfo::apply(VkImageViewCreateInfo& info)
{
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    info.viewType = viewType;
    info.format = format;
    info.components = components;
    info.subresourceRange = subresourceRange;
}

ImageView::ImageView(ref_ptr<CreateInfo> in_createInfo) :
    createInfo(in_createInfo)
{
    if (createInfo && createInfo->image)
    {
        _image = createInfo->image;
    }
}
ImageView::ImageView(Device* device, ref_ptr<CreateInfo> in_createInfo) :
    ImageView(in_createInfo)
{
    if (in_createInfo)
    {
        VkImageViewCreateInfo info = {};
        createInfo->apply(info);
        if (createInfo->image) info.image = in_createInfo->image->vk(device->deviceID);

        VulkanData& vd = _vulkanData[device->deviceID];
        vd.device = device;

        if (VkResult result = vkCreateImageView(*device, &info, device->getAllocationCallbacks(), &vd.imageView); result != VK_SUCCESS)
        {
            throw Exception{"Error: Failed to create ImageView.", result};
        }
    }
}


ImageView::~ImageView()
{
    for(auto& vd : _vulkanData) vd.release();
}

void ImageView::compile(Context& context)
{
    if (!createInfo) return;

    auto& vd = _vulkanData[context.deviceID];
    if (vd.imageView != VK_NULL_HANDLE) return;

    VkImageViewCreateInfo info = {};
    createInfo->apply(info);

    if (createInfo->image)
    {
        createInfo->image->compile(context);
        info.image = createInfo->image->vk(context.deviceID);
    }

    vd.device = context.device;

    if (VkResult result = vkCreateImageView(*vd.device, &info, vd.device->getAllocationCallbacks(), &vd.imageView); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create vkImage.", result};
    }
}

ref_ptr<ImageView> vsg::createImageView(vsg::Context& context, ref_ptr<Image::CreateInfo> imageCreateInfo, VkImageAspectFlags aspectFlags)
{
    vsg::Device* device = context.device;

    auto image = vsg::Image::create(device, imageCreateInfo);

    // get memory requirements
    VkMemoryRequirements memRequirements = image->getMemoryRequirements(device->deviceID);

    // allocate memory with out export memory info extension
    auto [deviceMemory, offset] = context.deviceMemoryBufferPools->reserveMemory(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (!deviceMemory)
    {
        throw Exception{"Error: Failed allocate memory for image.", 0};
    }

    image->bind(deviceMemory, offset);

    auto imageViewCreateInfo = vsg::ImageView::CreateInfo::create(image, aspectFlags);

    return vsg::ImageView::create(device, imageViewCreateInfo);
}

ref_ptr<ImageView> vsg::createImageView(Device* device, ref_ptr<Image::CreateInfo> imageCreateInfo, VkImageAspectFlags aspectFlags)
{
    auto image = vsg::Image::create(device, imageCreateInfo);

    // allocate memory with out export memory info extension
    auto deviceMemory = DeviceMemory::create(device, image->getMemoryRequirements(device->deviceID), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (!deviceMemory)
    {
        throw Exception{"Error: Failed allocate memory for image.", 0};
    }

    image->bind(deviceMemory, 0);

    auto imageViewCreateInfo = vsg::ImageView::CreateInfo::create(image, aspectFlags);
    return vsg::ImageView::create(device, imageViewCreateInfo);
}
