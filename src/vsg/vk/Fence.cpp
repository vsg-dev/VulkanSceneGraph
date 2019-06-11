/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Fence.h>

using namespace vsg;

Fence::Fence(VkFence fence, Device* device, AllocationCallbacks* allocator) :
    _vkFence(fence),
    _device(device),
    _allocator(allocator)
{
}

Fence::~Fence()
{
    if (_vkFence)
    {
        vkDestroyFence(*_device, _vkFence, _allocator);
    }
}

Fence::Result Fence::create(Device* device, VkFenceCreateFlags flags, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Result("Error: vsg::Fence::create(...) failed to create Fence, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkFenceCreateInfo createFenceInfo = {};
    createFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createFenceInfo.flags = flags;
    createFenceInfo.pNext = nullptr;

    VkFence fence;
    VkResult result = vkCreateFence(*device, &createFenceInfo, allocator, &fence);
    if (result == VK_SUCCESS)
    {
        return Result(new Fence(fence, device, allocator));
    }
    else
    {
        return Result("Error: Failed to create Fence.", result);
    }
}
