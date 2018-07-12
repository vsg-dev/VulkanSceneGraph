#include <vsg/vk/DeviceMemory.h>

#include <iostream>
#include <cstring>

namespace vsg
{

DeviceMemory::DeviceMemory(Device* device, VkDeviceMemory DeviceMemory, AllocationCallbacks* allocator) :
    _device(device),
    _deviceMemory(DeviceMemory),
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

DeviceMemory::Result DeviceMemory::create(PhysicalDevice* physicalDevice, Device* device, VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return DeviceMemory::Result("Error: vsg::DeviceMemory::create(...) failed to create DeviceMemory, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    uint32_t typeFilter = memRequirements.memoryTypeBits;

    // find the memory type to use
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &memProperties);
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
    VkResult result = vkAllocateMemory(*device, &allocateInfo, *allocator, &deviceMemory);
    if (result == VK_SUCCESS)
    {
        return new DeviceMemory(device, deviceMemory, allocator);
    }
    else
    {
        return Result("Error: Failed to create DeviceMemory.", result);
    }
}

void DeviceMemory::copy(VkDeviceSize offset, VkDeviceSize size, void* src_data)
{
    // should we have checks against buffer having enough memory for copied data?

    void* buffer_data;
    vkMapMemory(*_device, _deviceMemory, offset, size, 0, &buffer_data);

        std::memcpy(buffer_data, src_data, (size_t)size);

    vkUnmapMemory(*_device, _deviceMemory);
}


}