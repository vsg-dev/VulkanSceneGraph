#include <vsg/vk/Buffer.h>

#include <iostream>

namespace vsg
{

Buffer::Buffer(VkBuffer Buffer, VkBufferUsageFlags usage, VkSharingMode sharingMode, Device* device, AllocationCallbacks* allocator) :
    _buffer(Buffer),
    _usage(usage),
    _sharingMode(sharingMode),
    _device(device),
    _allocator(allocator)
{
}

Buffer::~Buffer()
{
    if (_buffer)
    {
        std::cout<<"Calling vkDestroyBuffer"<<std::endl;
        vkDestroyBuffer(*_device, _buffer, *_allocator);
    }
}

Buffer::Result Buffer::create(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Buffer::Result("Error: vsg::Buffer::create(...) failed to create vkBuffer, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = sharingMode;

    VkBuffer buffer;
    VkResult result = vkCreateBuffer(*device, &bufferInfo, *allocator, &buffer);
    if (result == VK_SUCCESS)
    {
        return new Buffer(buffer, usage, sharingMode, device, allocator);
    }
    else
    {
        return Result("Error: Failed to create vkBuffer.", result);
    }
}

}