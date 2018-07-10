#pragma once

#include <vsg/vk/Device.h>

namespace vsg
{
    class Semaphore : public Object
    {
    public:
        Semaphore(Device* device, VkSemaphore Semaphore, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Semaphore, VkResult, VK_SUCCESS>;
        static Result create(Device* device, AllocationCallbacks* allocator=nullptr);

        operator VkSemaphore () const { return _semaphore; }

    protected:
        virtual ~Semaphore();

        ref_ptr<Device>                 _device;
        VkSemaphore                     _semaphore;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
