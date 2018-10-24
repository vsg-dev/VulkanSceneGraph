/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/Image.h>

#include <cstring>

namespace vsg
{

DeviceMemory::DeviceMemory(VkDeviceMemory deviceMemory, Device* device, AllocationCallbacks* allocator) :
    _deviceMemory(deviceMemory),
    _device(device),
    _allocator(allocator)
{
}

DeviceMemory::~DeviceMemory()
{
    if (_deviceMemory)
    {
        vkFreeMemory(*_device, _deviceMemory, _allocator);
    }
}

DeviceMemory::Result DeviceMemory::create(Device* device, const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return DeviceMemory::Result("Error: vsg::DeviceMemory::create(...) failed to create DeviceMemory, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    uint32_t typeFilter = memRequirements.memoryTypeBits;

    // find the memory type to use
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(*(device->getPhysicalDevice()), &memProperties);
    uint32_t i;
    for (i=0; i< memProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1<<i)) && (memProperties.memoryTypes[i].propertyFlags & properties)==properties) break;
    }
    if (i>=memProperties.memoryTypeCount)
    {
        return DeviceMemory::Result("Error: vsg::DeviceMemory::create(...) failed to create DeviceMemory, not usage memory type found.", VK_ERROR_FORMAT_NOT_SUPPORTED);
    }
    uint32_t memoryTypeIndex = i;

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memRequirements.size;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;

    VkDeviceMemory deviceMemory;
    VkResult result = vkAllocateMemory(*device, &allocateInfo, allocator, &deviceMemory);
    if (result == VK_SUCCESS)
    {
        return Result(new DeviceMemory(deviceMemory, device, allocator));
    }
    else
    {
        return Result("Error: Failed to create DeviceMemory.", result);
    }
}

DeviceMemory::Result DeviceMemory::create(Device* device, Buffer* buffer, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator)
{
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*device, *buffer, &memRequirements);

    return vsg::DeviceMemory::create(device, memRequirements, properties, allocator);
}

DeviceMemory::Result DeviceMemory::create(Device* device, Image* image, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator)
{
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(*device, *image, &memRequirements);

    return vsg::DeviceMemory::create(device, memRequirements, properties, allocator);
}

VkResult DeviceMemory::map(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
{
    return vkMapMemory(*_device, _deviceMemory, offset, size, flags, ppData);
}

void DeviceMemory::unmap()
{
    vkUnmapMemory(*_device, _deviceMemory);
}



void DeviceMemory::copy(VkDeviceSize offset, VkDeviceSize size, void* src_data)
{
    // should we have checks against buffer having enough memory for copied data?

    void* buffer_data;
    map(offset, size, 0, &buffer_data);

        std::memcpy(buffer_data, src_data, (size_t)size);

    unmap();
}

void DeviceMemory::copy(VkDeviceSize offset, Data* data)
{
    copy(offset, data->dataSize(), data->dataPointer());
}


}
