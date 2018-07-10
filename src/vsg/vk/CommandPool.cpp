#include <vsg/vk/CommandPool.h>

#include <iostream>

namespace vsg
{

CommandPool::CommandPool(Device* device, VkCommandPool CommandPool, AllocationCallbacks* allocator) :
    _device(device),
    _commandPool(CommandPool),
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


    VkCommandPool commandPool;
    VkResult result = vkCreateCommandPool(*device, &poolInfo, *allocator, &commandPool);
    if (result == VK_SUCCESS)
    {
        return new CommandPool(device, commandPool, allocator);
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