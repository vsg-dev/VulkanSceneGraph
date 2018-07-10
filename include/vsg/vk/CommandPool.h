#pragma once

#include <vsg/vk/Device.h>
#include <vsg/vk/Swapchain.h>
#include <vsg/vk/RenderPass.h>

namespace vsg
{
    class CommandPool : public Object
    {
    public:
        CommandPool(Device* device, VkCommandPool CommandPool, AllocationCallbacks* allocator=nullptr);

        CommandPool(Device* device, uint32_t queueFamilyIndex, AllocationCallbacks* allocator=nullptr);

        operator VkCommandPool () const { return _commandPool; }

    protected:
        virtual ~CommandPool();

        ref_ptr<Device>                 _device;
        VkCommandPool                   _commandPool;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
