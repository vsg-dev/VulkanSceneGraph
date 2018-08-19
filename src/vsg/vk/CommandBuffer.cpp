#include <vsg/vk/CommandBuffer.h>

namespace vsg
{

CommandBuffer::CommandBuffer(Device* device, CommandPool* commandPool, VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags flags) :
    _commandBuffer(commandBuffer),
    _flags(flags),
    _device(device),
    _commandPool(commandPool)
{
}

CommandBuffer::~CommandBuffer()
{
    if (_commandBuffer)
    {
        vkFreeCommandBuffers((*_device), (*_commandPool), 1, &_commandBuffer);
    }
}

CommandBuffer::Result CommandBuffer::create(Device* device,  CommandPool* commandPool, VkCommandBufferUsageFlags flags)
{
    if (!device || !commandPool)
    {
        return CommandBuffer::Result("Error: vsg::CommandBuffer::create(...) failed to create command buffers, undefined Device or CommandPool.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = *commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer buffer;
    VkResult result = vkAllocateCommandBuffers(*device, &allocateInfo, &buffer);
    if (result == VK_SUCCESS)
    {
        return new CommandBuffer(device, commandPool, buffer, flags);
    }
    else
    {
        return Result("Error: Failed to create command buffers.", result);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// CommandBuffers
//
CommandBuffers::CommandBuffers(Device* device, CommandPool* commandPool, const Buffers& buffers) :
    _device(device),
    _commandPool(commandPool),
    _buffers(buffers)
{
}

CommandBuffers::Result CommandBuffers::create(Device* device,  CommandPool* commandPool, size_t size)
{
    if (!device || !commandPool)
    {
        return CommandBuffers::Result("Error: vsg::CommandBuffers::create(...) failed to create command buffers, undefined Device or CommandPool.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    Buffers buffers(size);

    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = *commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = (uint32_t) buffers.size();

    VkResult result = vkAllocateCommandBuffers(*device, &allocateInfo, buffers.data());
    if (result == VK_SUCCESS)
    {
        return new CommandBuffers(device, commandPool, buffers);
    }
    else
    {
        return Result("Error: Failed to create command buffers.", result);
    }
}

CommandBuffers::~CommandBuffers()
{
    if (!_buffers.empty())
    {
        vkFreeCommandBuffers(*_device, *_commandPool, static_cast<uint32_t>(_buffers.size()), _buffers.data());
    }
}

}
