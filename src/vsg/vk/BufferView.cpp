#include <vsg/vk/BufferView.h>

#include <iostream>

namespace vsg
{

BufferView::BufferView(Device* device, VkBufferView bufferView, Buffer* buffer, AllocationCallbacks* allocator) :
    _device(device),
    _bufferView(bufferView),
    _allocator(allocator),
    _buffer(buffer)
{
}

BufferView::~BufferView()
{
    if (_bufferView)
    {
        std::cout<<"Calling vkDestroyBufferView(..)"<<std::endl;
        vkDestroyBufferView(*_device, _bufferView, *_allocator);
    }
}

BufferView::Result BufferView::create(Device* device, Buffer* buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range, AllocationCallbacks* allocator)
{
    if (!device || !buffer)
    {
        return BufferView::Result("Error: vsg::BufferView::create(...) failed to create BufferView, device and/or buffer not defiend.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkBufferViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    createInfo.buffer = *buffer;
    createInfo.format = format;
    createInfo.offset = offset;
    createInfo.range = range;

    VkBufferView bufferView;
    VkResult result = vkCreateBufferView(*device, &createInfo, *allocator, &bufferView);
    if (result==VK_SUCCESS)
    {
        return new BufferView(device, bufferView, buffer, allocator);
    }
    else
    {
        return Result("Error: Failed to create BufferView.", result);
    }
}


}
