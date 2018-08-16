#include <vsg/vk/Semaphore.h>

namespace vsg
{

Semaphore::Semaphore(VkSemaphore semaphore, Device* device, AllocationCallbacks* allocator) :
    _semaphore(semaphore),
    _device(device),
    _allocator(allocator)
{
}

Semaphore::~Semaphore()
{
    if (_semaphore)
    {
        vkDestroySemaphore(*_device, _semaphore, _allocator);
    }
}

Semaphore::Result Semaphore::create(Device* device, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Semaphore::Result("Error: vsg::Semaphore::create(...) failed to create command pool, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaphore;
    VkResult result = vkCreateSemaphore(*device, &semaphoreInfo, allocator, &semaphore);
    if (result == VK_SUCCESS)
    {
        return new Semaphore(semaphore, device, allocator);
    }
    else
    {
        return Result("Error: Failed to create semaphore.", result);
    }
}

}
