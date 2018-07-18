#pragma once

#include <vsg/vk/Device.h>

namespace vsg
{

    class PipelineLayout : public vsg::Object
    {
    public:
        PipelineLayout(Device* device, VkPipelineLayout pipelineLayout, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<PipelineLayout, VkResult, VK_SUCCESS>;

        static Result create(Device* device, const VkPipelineLayoutCreateInfo& pipelineLayoutInfo, AllocationCallbacks* allocator=nullptr);

        static Result create(Device* device, AllocationCallbacks* allocator=nullptr);

        operator VkPipelineLayout () const { return _pipelineLayout; }

    protected:
        virtual ~PipelineLayout();

        ref_ptr<Device>                 _device;
        VkPipelineLayout                _pipelineLayout;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
