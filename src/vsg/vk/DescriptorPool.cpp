#include <vsg/vk/DescriptorPool.h>

namespace vsg
{

DescriptorPool::DescriptorPool(VkDescriptorPool descriptorPool, Device* device, AllocationCallbacks* allocator) :
    _descriptorPool(descriptorPool),
    _device(device),
    _allocator(allocator)
{
}

DescriptorPool::~DescriptorPool()
{
    if (_descriptorPool)
    {
        vkDestroyDescriptorPool(*_device, _descriptorPool, _allocator);
    }
}

DescriptorPool::Result DescriptorPool::create(Device* device, uint32_t maxSets, const DescriptorPoolSizes& descriptorPoolSizes, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Result("Error: vsg::DescriptorPool::create(...) failed to create DescriptorPool, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    if (descriptorPoolSizes.empty())
    {
        return Result("Error: vsg::DescriptorPool::create(...) failed to create DescriptorPool, descriptorPoolSizes.empty().", VK_ERROR_INITIALIZATION_FAILED);
    }

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = descriptorPoolSizes.size();
    poolInfo.pPoolSizes = descriptorPoolSizes.data();
    poolInfo.maxSets = maxSets;
    poolInfo.flags =VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // will we need VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT later?

    VkDescriptorPool descriptorPool;
    VkResult result = vkCreateDescriptorPool(*device, &poolInfo, allocator, &descriptorPool);
    if (result == VK_SUCCESS)
    {
        return new DescriptorPool(descriptorPool, device, allocator);
    }
    else
    {
        return Result("Error: Failed to create DescriptorPool.", result);
    }
}

}
