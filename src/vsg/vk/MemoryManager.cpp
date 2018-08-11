#include <vsg/vk/MemoryManager.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/Image.h>

#include <iostream>
#include <cstring>

namespace vsg
{

MemoryManager::MemoryManager(PhysicalDevice* physicalDevice, Device* device, AllocationCallbacks* allocator) :
    _physcicalDevice(physicalDevice),
    _device(device),
    _allocator(allocator)
{
}

MemoryManager::~MemoryManager()
{
}

Buffer* MemoryManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode)
{
    ref_ptr<Buffer> buffer = Buffer::create(_device, size, usage, sharingMode, _allocator);
    return buffer.release();
}

DeviceMemory* MemoryManager::createMemory(const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties)
{
    ref_ptr<DeviceMemory> memory = DeviceMemory::create(_physcicalDevice, _device, memRequirements, properties, _allocator);
    return memory.release();
}


}