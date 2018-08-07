#pragma once

#include <vsg/vk/Device.h>
#include <vsg/nodes/Group.h>

namespace vsg
{

    class RenderPass : public Object
    {
    public:
        RenderPass(Device* device, VkRenderPass renderPass, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<RenderPass, VkResult, VK_SUCCESS>;
        static Result create(Device* device, VkFormat imageFormat, VkFormat depthFormat, AllocationCallbacks* allocator=nullptr);

        operator VkRenderPass () const { return _renderPass; }

    protected:
        virtual ~RenderPass();

        ref_ptr<Device>                 _device;
        VkRenderPass                    _renderPass;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
