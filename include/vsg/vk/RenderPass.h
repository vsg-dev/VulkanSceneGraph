#pragma once

#include <vsg/vk/Device.h>
#include <vsg/nodes/Group.h>

namespace vsg
{

    class RenderPass : public Object
    {
    public:
        RenderPass(VkRenderPass renderPass, Device* device, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<RenderPass, VkResult, VK_SUCCESS>;
        static Result create(Device* device, VkFormat imageFormat, VkFormat depthFormat, AllocationCallbacks* allocator=nullptr);

        operator VkRenderPass () const { return _renderPass; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

    protected:
        virtual ~RenderPass();

        VkRenderPass                    _renderPass;
        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
