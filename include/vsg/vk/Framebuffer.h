#pragma once

#include <vsg/vk/Device.h>
#include <vsg/vk/Swapchain.h>
#include <vsg/vk/RenderPass.h>

namespace vsg
{
    class VSG_EXPORT Framebuffer : public Object
    {
    public:
        Framebuffer(VkFramebuffer framebuffer, Device* device, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<Framebuffer, VkResult, VK_SUCCESS>;
        static Result create(Device* device, VkFramebufferCreateInfo& framebufferInfo, AllocationCallbacks*  allocator=nullptr);

        operator VkFramebuffer () const { return _framebuffer; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~Framebuffer();

        VkFramebuffer                   _framebuffer;
        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
    };
}
