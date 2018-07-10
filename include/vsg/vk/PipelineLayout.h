#pragma once

#include <vsg/vk/Device.h>

namespace vsg
{

    class PipelineLayout : public vsg::Object
    {
    public:
        PipelineLayout(Device* device, VkPipelineLayout pipelineLayout, AllocationCallbacks* allocator=nullptr);

        PipelineLayout(Device* device, AllocationCallbacks* allocator=nullptr);

        operator VkPipelineLayout () const { return _pipelineLayout; }

    protected:
        virtual ~PipelineLayout();

        ref_ptr<Device>                 _device;
        VkPipelineLayout                _pipelineLayout;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
