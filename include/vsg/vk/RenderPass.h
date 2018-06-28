#pragma once

#include <vsg/vk/Device.h>

namespace vsg
{

    class RenderPass : public vsg::Object
    {
    public:
        RenderPass(Device* device, VkRenderPass renderPass, VkAllocationCallbacks* pAllocator=nullptr);

        RenderPass(Device* device, VkFormat imageFormat, VkAllocationCallbacks* pAllocator=nullptr);


        operator VkRenderPass () const { return _renderPass; }

    protected:
        virtual ~RenderPass();

        ref_ptr<Device>         _device;
        VkRenderPass            _renderPass;
        VkAllocationCallbacks*  _pAllocator;
    };

}
