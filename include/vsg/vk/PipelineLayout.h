#pragma once

#include <vsg/vk/Device.h>

namespace vsg
{

    class PipelineLayout : public vsg::Object
    {
    public:
        PipelineLayout(Device* device, VkPipelineLayout pipelineLayout, VkAllocationCallbacks* pAllocator=nullptr);

        PipelineLayout(Device* device, VkAllocationCallbacks* pAllocator=nullptr);

        operator VkPipelineLayout () const { return _pipelineLayout; }

    protected:
        virtual ~PipelineLayout();

        ref_ptr<Device>         _device;
        VkPipelineLayout        _pipelineLayout;
        VkAllocationCallbacks*  _pAllocator;
    };

}
