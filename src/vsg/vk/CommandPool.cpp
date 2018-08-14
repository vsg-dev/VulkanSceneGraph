#include <vsg/vk/CommandPool.h>

#include <iostream>

namespace vsg
{

CommandPool::CommandPool(VkCommandPool CommandPool, Device* device, AllocationCallbacks* allocator) :
    _commandPool(CommandPool),
    _device(device),
    _allocator(allocator)
{
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
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT || VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    //poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;


    VkCommandPool commandPool;
    VkResult result = vkCreateCommandPool(*device, &poolInfo, *allocator, &commandPool);
    if (result == VK_SUCCESS)
    {
        return new CommandPool(commandPool, device, allocator);
    }
    else
    {
        return Result("Error: Failed to create command pool.", result);
    }
}

CommandPool::~CommandPool()
{
    if (_commandPool)
    {
        std::cout<<"Calling vkDestroyCommandPool"<<std::endl;
        vkDestroyCommandPool(*_device, _commandPool, *_allocator);
    }
}

}