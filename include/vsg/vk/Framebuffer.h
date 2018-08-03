#pragma once

#include <vsg/vk/Device.h>
#include <vsg/vk/Swapchain.h>
#include <vsg/vk/RenderPass.h>

namespace vsg
{
    class Framebuffer : public Object
    {
    public:
        Framebuffer(Device* device, VkFramebuffer framebuffer, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Framebuffer, VkResult, VK_SUCCESS>;
        static Result create(Device* device, VkFramebufferCreateInfo& framebufferInfo, AllocationCallbacks*  allocator=nullptr);

        operator VkFramebuffer () const { return _framebuffer; }

    protected:
        virtual ~Framebuffer();

        ref_ptr<Device>                 _device;
        VkFramebuffer                   _framebuffer;
        ref_ptr<AllocationCallbacks>    _allocator;
    };
}
