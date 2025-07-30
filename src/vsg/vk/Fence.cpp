/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/vk/Fence.h>

using namespace vsg;

Fence::Fence(Device* device, VkFenceCreateFlags flags) :
    _device(device)
{
    VkFenceCreateInfo createFenceInfo = {};
    createFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createFenceInfo.flags = flags;
    createFenceInfo.pNext = nullptr;

    if (VkResult result = vkCreateFence(*device, &createFenceInfo, _device->getAllocationCallbacks(), &_vkFence); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create Fence.", result};
    }
}

Fence::~Fence()
{
    if (_vkFence)
    {
        vkDestroyFence(*_device, _vkFence, _device->getAllocationCallbacks());
    }
}

void Fence::resetFenceAndDependencies()
{
    for (auto& semaphore : _dependentSemaphores)
    {
        semaphore->numDependentSubmissions().exchange(0);
    }

    for (auto& commandBuffer : _dependentCommandBuffers)
    {
        commandBuffer->numDependentSubmissions().exchange(0);
    }

    _dependentSemaphores.clear();
    _dependentCommandBuffers.clear();

    reset();
}

VkResult Fence::wait(uint64_t timeout) const
{
    return vkWaitForFences(*_device, 1, &_vkFence, VK_TRUE, timeout);
}

VkResult Fence::reset() const
{
    return vkResetFences(*_device, 1, &_vkFence);
}
