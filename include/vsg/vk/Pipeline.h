#pragma once

#include <vsg/vk/Device.h>
#include <vsg/vk/Command.h>
#include <vsg/vk/PipelineLayout.h>

namespace vsg
{

    class Pipeline : public Command
    {
    public:
        Pipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint, Device* device, PipelineLayout* pipelineLayout, AllocationCallbacks* allocator=nullptr);

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        operator VkPipeline () const { return _pipeline; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

        PipelineLayout* getPipelineLayout() { return _pipelineLayout; }
        const PipelineLayout* getPipelineLayout() const { return _pipelineLayout; }

        virtual void dispatch(CommandBuffer& commandBuffer) const;

    protected:
        virtual ~Pipeline();

        VkPipeline                      _pipeline;
        VkPipelineBindPoint             _bindPoint;
        ref_ptr<Device>                 _device;
        ref_ptr<PipelineLayout>         _pipelineLayout;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
