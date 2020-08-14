/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Options.h>
#include <vsg/state/Image.h>
#include <vsg/vk/Context.h>

using namespace vsg;

void Image::VulkanData::release()
{
    if (image)
    {
        vkDestroyImage(*device, image, device->getAllocationCallbacks());
        image = VK_NULL_HANDLE;
    }

    if (deviceMemory)
    {
        deviceMemory->release(memoryOffset, 0); // TODO, we don't locally have a size allocated
        deviceMemory = {};
    }
}

Image::CreateInfo::CreateInfo(ref_ptr<Data> in_data) :
    data(in_data)
{
    if (data)
    {
        auto layout = data->getLayout();
        auto mipmapOffsets = data->computeMipmapOffsets();
        auto dimensions = data->dimensions();

        uint32_t width = data->width() * layout.blockWidth;
        uint32_t height = data->height() * layout.blockHeight;
        uint32_t depth = data->depth() * layout.blockDepth;

        imageType = dimensions >= 3 ? VK_IMAGE_TYPE_3D : (dimensions == 2 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D);
        format = layout.format;
        extent = VkExtent3D{width, height, depth};
        mipLevels = static_cast<uint32_t>(mipmapOffsets.size());
        arrayLayers = 1;
    }
}


void Image::CreateInfo::apply(VkImageCreateInfo& info)
{
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    info.imageType = imageType;
    info.format = format;
    info.extent = extent;
    info.mipLevels = mipLevels;
    info.arrayLayers = arrayLayers;
    info.samples = samples;
    info.tiling = tiling;
    info.usage = usage;
    info.sharingMode = sharingMode;
    info.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
    info.pQueueFamilyIndices = queueFamilyIndices.data();
    info.initialLayout = initialLayout;
}

Image::Image(ref_ptr<CreateInfo> in_createInfo) :
    createInfo(in_createInfo)
{
}

Image::Image(Device* device, ref_ptr<CreateInfo> in_createInfo) :
    createInfo(in_createInfo)
{
    if (in_createInfo)
    {

        VulkanData& vd = _vulkanData[device->deviceID];
        vd.device = device;

        VkImageCreateInfo info = {};
        createInfo->apply(info);

        if (VkResult result = vkCreateImage(*device, &info, device->getAllocationCallbacks(), &vd.image); result != VK_SUCCESS)
        {
            throw Exception{"Error: Failed to create vkImage.", result};
        }
    }
}

Image::Image(VkImage image, Device* device)
{
    VulkanData& vd = _vulkanData[device->deviceID];
    vd.image = image;
    vd.device = device;
}

Image::~Image()
{
    for(auto& vd : _vulkanData) vd.release();
}

VkResult Image::bind(DeviceMemory* deviceMemory, VkDeviceSize memoryOffset)
{
    VulkanData& vd = _vulkanData[deviceMemory->getDevice()->deviceID];

    VkResult result = vkBindImageMemory(*vd.device, vd.image, *deviceMemory, memoryOffset);
    if (result == VK_SUCCESS)
    {
        vd.deviceMemory = deviceMemory;
        vd.memoryOffset = memoryOffset;
    }
    return result;
}

VkMemoryRequirements Image::getMemoryRequirements(uint32_t deviceID) const
{
    const VulkanData& vd = _vulkanData[deviceID];

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(*vd.device, vd.image, &memRequirements);
    return memRequirements;
}

void Image::compile(Context& context)
{
    if (!createInfo) return;

    auto& vd = _vulkanData[context.deviceID];
    if (vd.image != VK_NULL_HANDLE) return;

    VkImageCreateInfo info = {};
    createInfo->apply(info);

    info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    vd.device = context.device;

    if (VkResult result = vkCreateImage(*vd.device, &info, vd.device->getAllocationCallbacks(), &vd.image); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create vkImage.", result};
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(*vd.device, vd.image, &memRequirements);

    auto [deviceMemory, offset] = context.deviceMemoryBufferPools->reserveMemory(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (!deviceMemory)
    {
        throw Exception{"Error: allocate memory to reserve slot.", VK_ERROR_OUT_OF_DEVICE_MEMORY};
    }

    bind(deviceMemory, offset);
}
