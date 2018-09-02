#pragma once

#include <vsg/vk/Device.h>

namespace vsg
{
    class VSG_EXPORT Semaphore : public Object
    {
    public:
        Semaphore(VkSemaphore Semaphore, Device* device, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Semaphore, VkResult, VK_SUCCESS>;
        static Result create(Device* device, AllocationCallbacks* allocator=nullptr);

        operator VkSemaphore () const { return _semaphore; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~Semaphore();

        VkSemaphore                     _semaphore;
        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
