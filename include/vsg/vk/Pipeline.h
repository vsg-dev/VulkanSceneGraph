#pragma once

#include <vsg/vk/PipelineLayout.h>
#include <vsg/nodes/StateGroup.h>

namespace vsg
{

    class VSG_EXPORT Pipeline : public Object
    {
    public:
        Pipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint, Device* device, PipelineLayout* pipelineLayout, AllocationCallbacks* allocator=nullptr);

        virtual void accept(Visitor& visitor) override { visitor.apply(*this); }

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

    class VSG_EXPORT BindPipeline : public StateComponent
    {
    public:
        BindPipeline(Pipeline* pipeline);

        Pipeline* getPipeline() { return _pipeline; }
        const Pipeline* getPipeline() const { return _pipeline; }

        virtual void pushTo(State& state) override;
        virtual void popFrom(State& state) override;
        virtual void dispatch(CommandBuffer& commandBuffer) const override;

    public:
        virtual ~BindPipeline();

        ref_ptr<Pipeline> _pipeline;
    };

}
