#include <vsg/vk/CommandPool.h>

#include <iostream>

namespace vsg
{

CommandPool::CommandPool(Device* device, VkCommandPool CommandPool, VkAllocationCallbacks* pAllocator) :
    _device(device),
    _commandPool(CommandPool),
    _pAllocator(pAllocator)
{
}

CommandPool::CommandPool(Device* device, uint32_t queueFamilyIndex, VkAllocationCallbacks* pAllocator) :
    _device(device),
    _commandPool(VK_NULL_HANDLE),
    _pAllocator(pAllocator)
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    if (vkCreateCommandPool(*device, &poolInfo, pAllocator, &_commandPool) == VK_SUCCESS)
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
        vkDestroyCommandPool(*_device, _commandPool, _pAllocator);
    }
}

}