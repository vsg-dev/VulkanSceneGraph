#pragma once

#include <vsg/vk/Command.h>
#include <vsg/vk/Device.h>

namespace vsg
{

    class Pipeline : public Command
    {
    public:
        Pipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint, Device* device, AllocationCallbacks* allocator=nullptr);

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        operator VkPipeline () const { return _pipeline; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

        virtual void dispatch(VkCommandBuffer commandBuffer) const
        {
            vkCmdBindPipeline(commandBuffer, _bindPoint, _pipeline);
        }

    protected:
        virtual ~Pipeline();

        VkPipeline                      _pipeline;
        VkPipelineBindPoint             _bindPoint;
        ref_ptr<Device>                 _device;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
