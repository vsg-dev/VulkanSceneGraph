#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/PipelineLayout.h>
#include <vsg/nodes/StateGroup.h>

namespace vsg
{

    class VSG_EXPORT Pipeline : public Inherit<Object, Pipeline>
    {
    public:
        Pipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint, Device* device, PipelineLayout* pipelineLayout, AllocationCallbacks* allocator=nullptr);

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

    class VSG_EXPORT BindPipeline : public Inherit<StateComponent, BindPipeline>
    {
    public:
        BindPipeline(Pipeline* pipeline);

        Pipeline* getPipeline() { return _pipeline; }
        const Pipeline* getPipeline() const { return _pipeline; }

        void pushTo(State& state) override;
        void popFrom(State& state) override;
        void dispatch(CommandBuffer& commandBuffer) const override;

    public:
        virtual ~BindPipeline();

        ref_ptr<Pipeline> _pipeline;
    };

}
