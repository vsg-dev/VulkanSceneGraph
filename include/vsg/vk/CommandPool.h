#pragma once

#include <vsg/vk/Device.h>
#include <vsg/vk/Swapchain.h>
#include <vsg/vk/RenderPass.h>

namespace vsg
{
    class CommandPool : public Object
    {
    public:
        CommandPool(VkCommandPool CommandPool, Device* device, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<CommandPool, VkResult, VK_SUCCESS>;
        static Result create(Device* device, uint32_t queueFamilyIndex, AllocationCallbacks* allocator=nullptr);

        operator VkCommandPool () const { return _commandPool; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~CommandPool();

        VkCommandPool                   _commandPool;
        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
