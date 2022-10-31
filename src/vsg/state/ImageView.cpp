/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/io/Options.h>
#include <vsg/state/ImageView.h>
#include <vsg/vk/Context.h>

using namespace vsg;

VkImageAspectFlags vsg::computeAspectFlagsForFormat(VkFormat format)
{
    if (format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT)
    {
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    else if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_X8_D24_UNORM_PACK32)
    {
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else
    {
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

void ImageView::VulkanData::release()
{
    if (imageView)
    {
        vkDestroyImageView(*device, imageView, device->getAllocationCallbacks());
        imageView = VK_NULL_HANDLE;
        device = {};
    }
}

ImageView::ImageView(ref_ptr<Image> in_image) :
    image(in_image)
{
    if (image)
    {
        if (image->data && image->data->properties.imageViewType >= 0)
        {
            viewType = static_cast<VkImageViewType>(image->data->properties.imageViewType);
        }
        else
        {
            auto imageType = image->imageType;
            viewType = (imageType == VK_IMAGE_TYPE_3D) ? VK_IMAGE_VIEW_TYPE_3D : ((imageType == VK_IMAGE_TYPE_2D) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_1D);
        }

        format = image->format;
        subresourceRange.aspectMask = computeAspectFlagsForFormat(image->format);
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = image->mipLevels;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = image->arrayLayers;
    }
}

ImageView::ImageView(ref_ptr<Image> in_image, VkImageAspectFlags aspectFlags) :
    image(in_image)
{
    if (image)
    {
        if (image->data && image->data->properties.imageViewType >= 0)
        {
            viewType = static_cast<VkImageViewType>(image->data->properties.imageViewType);
        }
        else
        {
            auto imageType = image->imageType;
            viewType = (imageType == VK_IMAGE_TYPE_3D) ? VK_IMAGE_VIEW_TYPE_3D : ((imageType == VK_IMAGE_TYPE_2D) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_1D);
        }

        format = image->format;
        subresourceRange.aspectMask = aspectFlags;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = image->mipLevels;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = image->arrayLayers;
    }
}

ImageView::~ImageView()
{
    for (auto& vd : _vulkanData) vd.release();
}

int ImageView::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);

    if ((result = compare_value(flags, rhs.flags))) return result;
    if ((result = compare_pointer(image, rhs.image))) return result;
    if ((result = compare_value(viewType, rhs.viewType))) return result;
    if ((result = compare_memory(components, rhs.components))) return result;
    return compare_memory(subresourceRange, rhs.subresourceRange);
}

void ImageView::compile(Device* device)
{
    auto& vd = _vulkanData[device->deviceID];
    if (vd.imageView != VK_NULL_HANDLE) return;

    vd.device = device;

    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    info.viewType = viewType;
    info.format = format;
    info.components = components;
    info.subresourceRange = subresourceRange;

    if (image)
    {
        image->compile(device);

        info.image = image->vk(device->deviceID);
    }

    if (VkResult result = vkCreateImageView(*vd.device, &info, vd.device->getAllocationCallbacks(), &vd.imageView); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create vkImageView.", result};
    }
}

void ImageView::compile(Context& context)
{
    auto& vd = _vulkanData[context.deviceID];
    if (vd.imageView != VK_NULL_HANDLE) return;

    vd.device = context.device;

    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    info.viewType = viewType;
    info.format = format;
    info.components = components;
    info.subresourceRange = subresourceRange;

    if (image)
    {
        image->compile(context);

        info.image = image->vk(vd.device->deviceID);
    }

    if (VkResult result = vkCreateImageView(*vd.device, &info, vd.device->getAllocationCallbacks(), &vd.imageView); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create vkImageView.", result};
    }
}

ref_ptr<ImageView> vsg::createImageView(vsg::Context& context, ref_ptr<Image> image, VkImageAspectFlags aspectFlags)
{
    vsg::Device* device = context.device;

    image->compile(device);

    // get memory requirements
    VkMemoryRequirements memRequirements = image->getMemoryRequirements(device->deviceID);

    // allocate memory with out export memory info extension
    auto [deviceMemory, offset] = context.deviceMemoryBufferPools->reserveMemory(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    image->bind(deviceMemory, offset);

    auto imageView = ImageView::create(image, aspectFlags);
    imageView->compile(device);

    return imageView;
}

ref_ptr<ImageView> vsg::createImageView(Device* device, ref_ptr<Image> image, VkImageAspectFlags aspectFlags)
{
    image->compile(device);

    image->allocateAndBindMemory(device);

    auto imageView = ImageView::create(image, aspectFlags);
    imageView->compile(device);

    return imageView;
}
