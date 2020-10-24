/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Options.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

CommandBuffer::CommandBuffer(Device* device, CommandPool* commandPool, VkCommandBufferLevel level) :
    deviceID(device->deviceID),
    scratchMemory(ScratchMemory::create(4096)),
    _level(level),
    _device(device),
    _commandPool(commandPool),
    _currentPipelineLayout(VK_NULL_HANDLE),
    _currentPushConstantStageFlags(0)
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = *commandPool;
    allocateInfo.level = level;
    allocateInfo.commandBufferCount = 1;

    if (VkResult result = vkAllocateCommandBuffers(*device, &allocateInfo, &_commandBuffer); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create command buffers.", result};
    }
}

CommandBuffer::~CommandBuffer()
{
    if (_commandBuffer)
    {
        vkFreeCommandBuffers((*_device), (*_commandPool), 1, &_commandBuffer);
    }
}
