/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Options.h>
#include <vsg/state/Image.h>

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

Image::Image(VkImage image, Device* device)
{
    VulkanData& vd = _vulkanData[device->deviceID];
    vd.image = image;
    vd.device = device;
}

Image::Image(Device* device, const VkImageCreateInfo& createImageInfo)
{
    VulkanData& vd = _vulkanData[device->deviceID];
    vd.device = device;

    if (VkResult result = vkCreateImage(*device, &createImageInfo, device->getAllocationCallbacks(), &vd.image); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create vkImage.", result};
    }
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
