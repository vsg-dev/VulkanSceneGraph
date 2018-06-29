#pragma once

#include <vsg/vk/Device.h>
#include <vsg/vk/Swapchain.h>
#include <vsg/vk/RenderPass.h>

namespace vsg
{
    class CommandPool : public Object
    {
    public:
        CommandPool(Device* device, VkCommandPool CommandPool, VkAllocationCallbacks* pAllocator=nullptr);

        CommandPool(Device* device, uint32_t queueFamilyIndex, VkAllocationCallbacks* pAllocator=nullptr);

        operator VkCommandPool () const { return _commandPool; }

    protected:
        virtual ~CommandPool();

        ref_ptr<Device>         _device;
        VkCommandPool           _commandPool;
        VkAllocationCallbacks*  _pAllocator;
    };

}
