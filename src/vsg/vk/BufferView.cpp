#include <vsg/vk/BufferView.h>

namespace vsg
{

BufferView::BufferView(VkBufferView bufferView, Device* device, Buffer* buffer, AllocationCallbacks* allocator) :
    _bufferView(bufferView),
    _device(device),
    _buffer(buffer),
    _allocator(allocator)
{
}

BufferView::~BufferView()
{
    if (_bufferView)
    {
        vkDestroyBufferView(*_device, _bufferView, _allocator);
    }
}

BufferView::Result BufferView::create(Buffer* buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range, AllocationCallbacks* allocator)
{
    if (!buffer)
    {
        return BufferView::Result("Error: vsg::BufferView::create(...) failed to create BufferView, buffer not defiend.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkBufferViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    createInfo.buffer = *buffer;
    createInfo.format = format;
    createInfo.offset = offset;
    createInfo.range = range;

    VkBufferView bufferView;
    VkResult result = vkCreateBufferView(*(buffer->getDevice()), &createInfo, allocator, &bufferView);
    if (result==VK_SUCCESS)
    {
        return new BufferView(bufferView, buffer->getDevice(), buffer, allocator);
    }
    else
    {
        return Result("Error: Failed to create BufferView.", result);
    }
}


}
