/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BufferView.h>

using namespace vsg;

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
        return Result("Error: vsg::BufferView::create(...) failed to create BufferView, buffer not defined.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkBufferViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    createInfo.buffer = *buffer;
    createInfo.format = format;
    createInfo.offset = offset;
    createInfo.range = range;
    createInfo.pNext = nullptr;

    VkBufferView bufferView;
    VkResult result = vkCreateBufferView(*(buffer->getDevice()), &createInfo, allocator, &bufferView);
    if (result == VK_SUCCESS)
    {
        return Result(new BufferView(bufferView, buffer->getDevice(), buffer, allocator));
    }
    else
    {
        return Result("Error: Failed to create BufferView.", result);
    }
}
