#pragma once

#include <vsg/vk/Command.h>
#include <vsg/vk/Device.h>

namespace vsg
{

    class Pipeline : public Command
    {
    public:
        Pipeline(Device* device, VkPipeline pipeline, VkPipelineBindPoint bindPoint, AllocationCallbacks* allocator=nullptr);

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        operator VkPipeline () const { return _pipeline; }

        virtual void dispatch(VkCommandBuffer commandBuffer) const
        {
            vkCmdBindPipeline(commandBuffer, _bindPoint, _pipeline);
        }

    protected:
        virtual ~Pipeline();

        ref_ptr<Device>                 _device;
        VkPipeline                      _pipeline;
        VkPipelineBindPoint             _bindPoint;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
