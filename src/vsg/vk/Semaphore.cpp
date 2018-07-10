#include <vsg/vk/Semaphore.h>

#include <iostream>

namespace vsg
{

Semaphore::Semaphore(Device* device, VkSemaphore semaphore, AllocationCallbacks* allocator) :
    _device(device),
    _semaphore(semaphore),
    _allocator(allocator)
{
}

Semaphore::~Semaphore()
{
    if (_semaphore)
    {
        std::cout<<"Calling vkDestroySemaphore"<<std::endl;
        vkDestroySemaphore(*_device, _semaphore, *_allocator);
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
    VkResult result = vkCreateSemaphore(*device, &semaphoreInfo, *allocator, &semaphore);
    if (result == VK_SUCCESS)
    {
        return new Semaphore(device, semaphore, allocator);
    }
    else
    {
        return Result("Error: Failed to create semaphore.", result);
    }
}

}