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

CommandPool::CommandPool(Device* device, uint32_t queueFamilyIndex, AllocationCallbacks* allocator) :
    _device(device),
    _commandPool(VK_NULL_HANDLE),
    _allocator(allocator)
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    if (vkCreateCommandPool(*device, &poolInfo, *allocator, &_commandPool) == VK_SUCCESS)
    {
        std::cout<<"Created CommandPool"<<std::endl;
    }
    else
    {
        std::cout<<"Warning: unable to create CommandPool"<<std::endl;
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