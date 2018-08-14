#include <vsg/vk/Fence.h>

#include <iostream>

namespace vsg
{

Fence::Fence(VkFence Fence, Device* device, AllocationCallbacks* allocator) :
    _vkFence(Fence),
    _device(device),
    _allocator(allocator)
{
}

Fence::~Fence()
{
    if (_vkFence)
    {
        std::cout<<"Calling vkDestroyFence"<<std::endl;
        vkDestroyFence(*_device, _vkFence, *_allocator);
    }
}

Fence::Result Fence::create(Device* device, VkFenceCreateFlags flags, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Fence::Result("Error: vsg::Fence::create(...) failed to create Fence, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkFenceCreateInfo createFenceInfo = {};
    createFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createFenceInfo.flags = flags;

    VkFence fence;
    VkResult result = vkCreateFence(*device, &createFenceInfo, *allocator, &fence);
    if (result == VK_SUCCESS)
    {
        return new Fence(fence, device, allocator);
    }
    else
    {
        return Result("Error: Failed to create Fence.", result);
    }
}

}