#pragma once

#include <vsg/vk/Device.h>

namespace vsg
{
    class Fence : public Object
    {
    public:
        Fence(VkFence Fence, Device* device, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Fence, VkResult, VK_SUCCESS>;
        static Result create(Device* device, VkFenceCreateFlags flags=0, AllocationCallbacks* allocator=nullptr);

        VkResult wait(uint64_t timeout) const { return vkWaitForFences(*_device, 1, &_vkFence, VK_TRUE, timeout); }

        operator VkFence() const{ return (this!=nullptr) ? _vkFence : VK_NULL_HANDLE; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~Fence();

        VkFence                         _vkFence;
        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
