#include <vsg/vk/DeviceMemory.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/Image.h>

#include <iostream>
#include <cstring>

namespace vsg
{

DeviceMemory::DeviceMemory(VkDeviceMemory DeviceMemory, Device* device, AllocationCallbacks* allocator) :
    _deviceMemory(DeviceMemory),
    _device(device),
    _allocator(allocator)
{
}

DeviceMemory::~DeviceMemory()
{
    if (_deviceMemory)
    {
        std::cout<<"Calling vkFreeMemory"<<std::endl;
        vkFreeMemory(*_device, _deviceMemory, *_allocator);
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

    std::cout<<"vkAllocateMemory(...) allocateInfo.allocationSize="<<allocateInfo.allocationSize<<std::endl;

    VkDeviceMemory deviceMemory;
    VkResult result = vkAllocateMemory(*device, &allocateInfo, *allocator, &deviceMemory);
    if (result == VK_SUCCESS)
    {
        return new DeviceMemory(deviceMemory, device, allocator);
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

    std::cout<<"DeviceMemory::copy("<<offset<<", "<<size<<", "<<src_data<<")"<<std::endl;

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