/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/vk/BufferView.h>
#include <vsg/io/Options.h>

using namespace vsg;

BufferView::BufferView(Buffer* buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range, AllocationCallbacks* allocator) :
    _device(buffer->getDevice()),
    _buffer(buffer),
    _allocator(allocator)
{
    VkBufferViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    createInfo.buffer = *buffer;
    createInfo.format = format;
    createInfo.offset = offset;
    createInfo.range = range;
    createInfo.pNext = nullptr;

    if (VkResult result = vkCreateBufferView(*(buffer->getDevice()), &createInfo, allocator, &_bufferView); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create BufferView.", result};
    }
}

BufferView::~BufferView()
{
    if (_bufferView)
    {
        vkDestroyBufferView(*_device, _bufferView, _allocator);
    }
}
