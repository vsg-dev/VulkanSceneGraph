#pragma once

#include <vsg/vk/Device.h>
#include <vsg/vk/Swapchain.h>
#include <vsg/vk/RenderPass.h>

namespace vsg
{
    class Framebuffer : public Object
    {
    public:
        Framebuffer(Device* device, VkFramebuffer framebuffer, VkAllocationCallbacks* pAllocator=nullptr);

        operator VkFramebuffer () const { return _framebuffer; }

    protected:
        virtual ~Framebuffer();

        ref_ptr<Device>         _device;
        VkFramebuffer           _framebuffer;
        VkAllocationCallbacks*  _pAllocator;
    };

    using Framebuffers = std::vector<ref_ptr<Framebuffer>>;

extern Framebuffers createFrameBuffers(Device* device, Swapchain* swapchain, RenderPass* renderPass, VkAllocationCallbacks* pAllocator=nullptr);


}
