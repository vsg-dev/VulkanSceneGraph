#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Command.h>
#include <vsg/vk/PipelineLayout.h>
#include <vsg/vk/ShaderModule.h>

namespace vsg
{

    class VSG_DECLSPEC ComputePipeline : public Inherit<Object, ComputePipeline>
    {
    public:

        ComputePipeline(PipelineLayout* pipelineLayout, ShaderModule* shaderModule, AllocationCallbacks* allocator = nullptr);

        PipelineLayout* getPipelineLayout() { return _pipelineLayout; }
        const PipelineLayout* getPipelineLayout() const { return _pipelineLayout; }

        ShaderModule* getShaderModule() { return _shaderModule; }
        const ShaderModule* getShaderModule() const { return _shaderModule; }

        class VSG_DECLSPEC Implementation : public Inherit<Object, Implementation>
        {
        public:
            Implementation(VkPipeline pipeline, Device* device, PipelineLayout* pipelineLayout, ShaderModule* shaderModule, AllocationCallbacks* allocator);
            virtual ~Implementation();

            using Result = vsg::Result<Implementation, VkResult, VK_SUCCESS>;

            /** Create a ComputePipeline.*/
            static Result create(Device* device, PipelineLayout* pipelineLayout, ShaderModule* shaderModule, AllocationCallbacks* allocator = nullptr);

            VkPipeline _pipeline;

            ref_ptr<Device> _device;
            ref_ptr<PipelineLayout> _pipelineLayout;
            ref_ptr<ShaderModule> _shaderModule;
            ref_ptr<AllocationCallbacks> _allocator;
        };

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release() { _implementation = nullptr; }

        operator VkPipeline() const { return _implementation->_pipeline; }

    protected:

        virtual ~ComputePipeline();

        ref_ptr<PipelineLayout> _pipelineLayout;
        ref_ptr<ShaderModule> _shaderModule;
        ref_ptr<AllocationCallbacks> _allocator;

        ref_ptr<Implementation> _implementation;
    };

    class VSG_DECLSPEC BindComputePipeline : public Inherit<StateCommand, BindComputePipeline>
    {
    public:
        BindComputePipeline(ComputePipeline* pipeline);

        void setPipeline(ComputePipeline* pipeline) { _pipeline = pipeline; }
        ComputePipeline* getPipeline() { return _pipeline; }
        const ComputePipeline* getPipeline() const { return _pipeline; }

        void pushTo(State& state) const override;
        void popFrom(State& state) const override;
        void dispatch(CommandBuffer& commandBuffer) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context) override;

    public:
        virtual ~BindComputePipeline();

        ref_ptr<ComputePipeline> _pipeline;
    };
    VSG_type_name(vsg::BindComputePipeline);

} // namespace vsg
