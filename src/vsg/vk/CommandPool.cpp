/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/CommandPool.h>

using namespace vsg;

CommandPool::CommandPool(VkCommandPool commandPool, Device* device, AllocationCallbacks* allocator) :
    _commandPool(commandPool),
    _device(device),
    _allocator(allocator)
{
}

CommandPool::~CommandPool()
{
    if (_commandPool)
    {
        vkDestroyCommandPool(*_device, _commandPool, _allocator);
    }
}

CommandPool::Result CommandPool::create(Device* device, uint32_t queueFamilyIndex, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return CommandPool::Result("Error: vsg::CommandPool::create(...) failed to create command pool, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    //poolInfo.flags = 0;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    //poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.pNext = nullptr;

    VkCommandPool commandPool;
    VkResult result = vkCreateCommandPool(*device, &poolInfo, allocator, &commandPool);
    if (result == VK_SUCCESS)
    {
        return Result(new CommandPool(commandPool, device, allocator));
    }
    else
    {
        return Result("Error: Failed to create command pool.", result);
    }
}
