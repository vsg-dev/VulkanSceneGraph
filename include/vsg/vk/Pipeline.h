#pragma once

#include <vsg/vk/Device.h>
#include <vsg/vk/Command.h>
#include <vsg/vk/PipelineLayout.h>

namespace vsg
{

    class Pipeline : public Object
    {
    public:
        Pipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint, Device* device, PipelineLayout* pipelineLayout, AllocationCallbacks* allocator=nullptr);

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        operator VkPipeline () const { return _pipeline; }

        VkPipelineBindPoint getBindPoint() const { return _bindPoint; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

        PipelineLayout* getPipelineLayout() { return _pipelineLayout; }
        const PipelineLayout* getPipelineLayout() const { return _pipelineLayout; }

    protected:
        virtual ~Pipeline();

        VkPipeline                      _pipeline;
        VkPipelineBindPoint             _bindPoint;
        ref_ptr<Device>                 _device;
        ref_ptr<PipelineLayout>         _pipelineLayout;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

    class BindPipeline : public Command
    {
    public:
        BindPipeline(Pipeline* pipeline);

        Pipeline* getPipeline() { return _pipeline; }
        const Pipeline* getPipeline() const { return _pipeline; }

        virtual void dispatch(CommandBuffer& commandBuffer) const;

    public:
        virtual ~BindPipeline();

        ref_ptr<Pipeline> _pipeline;
    };

}
