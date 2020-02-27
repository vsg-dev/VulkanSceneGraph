#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Command.h>
#include <vsg/vk/PipelineLayout.h>
#include <vsg/vk/ShaderStage.h>

namespace vsg
{

    class VSG_DECLSPEC ComputePipeline : public Inherit<Object, ComputePipeline>
    {
    public:
        ComputePipeline();
        ComputePipeline(PipelineLayout* pipelineLayout, ShaderStage* shaderStage, AllocationCallbacks* allocator = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        PipelineLayout* getPipelineLayout() { return _pipelineLayout; }
        const PipelineLayout* getPipelineLayout() const { return _pipelineLayout; }

        ShaderStage* getShaderStage() { return _shaderStage; }
        const ShaderStage* getShaderStage() const { return _shaderStage; }

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release(uint32_t deviceID) { _implementation[deviceID] = nullptr; }
        void release()
        {
            for (auto& imp : _implementation) imp = nullptr;
        }

        VkPipeline vk(uint32_t deviceID) const { return _implementation[deviceID]->vk(); }

    protected:
        virtual ~ComputePipeline();

        class VSG_DECLSPEC Implementation : public Inherit<Object, Implementation>
        {
        public:
            Implementation(VkPipeline pipeline, Device* device, PipelineLayout* pipelineLayout, ShaderStage* shaderStage, AllocationCallbacks* allocator);
            virtual ~Implementation();

            using Result = vsg::Result<Implementation, VkResult, VK_SUCCESS>;

            /** Create a ComputePipeline.*/
            static Result create(Device* device, PipelineLayout* pipelineLayout, ShaderStage* shaderStage, AllocationCallbacks* allocator = nullptr);

            VkPipeline vk() const { return _pipeline; }

            VkPipeline _pipeline;

            ref_ptr<Device> _device;
            ref_ptr<PipelineLayout> _pipelineLayout;
            ref_ptr<ShaderStage> _shaderStage;
            ref_ptr<AllocationCallbacks> _allocator;
        };

        vk_buffer<ref_ptr<Implementation>> _implementation;

        ref_ptr<PipelineLayout> _pipelineLayout;
        ref_ptr<ShaderStage> _shaderStage;
        ref_ptr<AllocationCallbacks> _allocator;
    };

    class VSG_DECLSPEC BindComputePipeline : public Inherit<StateCommand, BindComputePipeline>
    {
    public:
        BindComputePipeline(ComputePipeline* pipeline = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        void setPipeline(ComputePipeline* pipeline) { _pipeline = pipeline; }
        ComputePipeline* getPipeline() { return _pipeline; }
        const ComputePipeline* getPipeline() const { return _pipeline; }

        void dispatch(CommandBuffer& commandBuffer) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context) override;

    public:
        virtual ~BindComputePipeline();

        ref_ptr<ComputePipeline> _pipeline;
    };
    VSG_type_name(vsg::BindComputePipeline);

} // namespace vsg
