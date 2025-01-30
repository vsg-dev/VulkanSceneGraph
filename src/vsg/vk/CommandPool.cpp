/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/CommandPool.h>

using namespace vsg;

CommandPool::CommandPool(Device* device, uint32_t in_queueFamilyIndex, VkCommandPoolCreateFlags in_flags) :
    queueFamilyIndex(in_queueFamilyIndex),
    flags(in_flags),
    _device(device)
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    poolInfo.flags = flags;
    poolInfo.pNext = nullptr;

    if (VkResult result = vkCreateCommandPool(*device, &poolInfo, _device->getAllocationCallbacks(), &_commandPool); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create command pool.", result};
    }
}

CommandPool::~CommandPool()
{
    if (_commandPool)
    {
        vkDestroyCommandPool(*_device, _commandPool, _device->getAllocationCallbacks());
    }
}

ref_ptr<CommandBuffer> CommandPool::allocate(VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = _commandPool;
    allocateInfo.level = level;
    allocateInfo.commandBufferCount = 1;

    std::scoped_lock<std::mutex> lock(_mutex);
    VkCommandBuffer commandBuffer;
    if (VkResult result = vkAllocateCommandBuffers(*_device, &allocateInfo, &commandBuffer); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create command buffers.", result};
    }

    return ref_ptr<CommandBuffer>(new CommandBuffer(this, commandBuffer, level));
}

void CommandPool::free(CommandBuffer* commandBuffer)
{
    if (commandBuffer && commandBuffer->_commandBuffer)
    {
        std::scoped_lock<std::mutex> lock(_mutex);
        vkFreeCommandBuffers(*_device, _commandPool, 1, commandBuffer->data());
        commandBuffer->_commandBuffer = 0;
    }
}
