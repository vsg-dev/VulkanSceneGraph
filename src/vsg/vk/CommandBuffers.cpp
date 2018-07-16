#include <vsg/vk/CommandBuffers.h>

#include <iostream>

namespace vsg
{

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
        std::cout<<"Calling vkFreeCommandBuffers"<<std::endl;
        vkFreeCommandBuffers(*_device, *_commandPool, static_cast<uint32_t>(_buffers.size()), _buffers.data());
    }
}

}