#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/raytracing/RayTracingShaderGroup.h>
#include <vsg/vk/Command.h>
#include <vsg/vk/PipelineLayout.h>
#include <vsg/vk/ShaderStage.h>

namespace vsg
{
    class VSG_DECLSPEC RayTracingPipeline : public Inherit<Object, RayTracingPipeline>
    {
    public:
        RayTracingPipeline();

        RayTracingPipeline(PipelineLayout* pipelineLayout, const ShaderStages& shaderStages, const RayTracingShaderGroups& shaderGroups, AllocationCallbacks* allocator = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        PipelineLayout* getPipelineLayout() { return _pipelineLayout; }
        const PipelineLayout* getPipelineLayout() const { return _pipelineLayout; }

        ShaderStages& getShaderStages() { return _shaderStages; }
        const ShaderStages& getShaderStages() const { return _shaderStages; }

        RayTracingShaderGroups& getRayTracingShaderGroups() { return _rayTracingShaderGroups; }
        const RayTracingShaderGroups& getRayTracingShaderGroups() const { return _rayTracingShaderGroups; }

        AllocationCallbacks* getAllocationCallbacks() { return _allocator; }
        const AllocationCallbacks* getAllocationCallbacks() const { return _allocator; }

        uint32_t& maxRecursionDepth() { return _maxRecursionDepth; }
        const uint32_t& maxRecursionDepth() const { return _maxRecursionDepth; }

        class VSG_DECLSPEC Implementation : public Inherit<Object, Implementation>
        {
        public:
            Implementation(VkPipeline pipeline, Device* device, RayTracingPipeline* rayTracingPipeline, AllocationCallbacks* allocator = nullptr);
            virtual ~Implementation();

            using Result = vsg::Result<Implementation, VkResult, VK_SUCCESS>;

            /** Create a GraphicsPipeline.*/
            static Result create(Context& context, RayTracingPipeline* rayTracingPipeline);

            VkPipeline _pipeline;

            // TODO need to convert to use Implementation versions of RenderPass and PipelineLayout
            ref_ptr<Device> _device;
            ref_ptr<PipelineLayout> _pipelineLayout;
            ShaderStages _shaderStages;
            RayTracingShaderGroups _shaderGroups;
            ref_ptr<AllocationCallbacks> _allocator;
        };

        // get the Vulkan GrphicsPipeline::Implementation
        Implementation* getImplementation() { return _implementation; }
        const Implementation* getImplementation() const { return _implementation; }

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release() { _implementation = nullptr; }

        operator VkPipeline() const { return _implementation->_pipeline; }

    protected:
        virtual ~RayTracingPipeline();

        ref_ptr<Device> _device;
        ref_ptr<PipelineLayout> _pipelineLayout;
        ShaderStages _shaderStages;
        RayTracingShaderGroups _rayTracingShaderGroups;
        uint32_t _maxRecursionDepth = 1;

        ref_ptr<AllocationCallbacks> _allocator;

        ref_ptr<Implementation> _implementation;
    };
    VSG_type_name(vsg::RayTracingPipeline);

    class VSG_DECLSPEC BindRayTracingPipeline : public Inherit<StateCommand, BindRayTracingPipeline>
    {
    public:
        BindRayTracingPipeline(RayTracingPipeline* pipeline = nullptr);

        void read(Input& input) override;
        void write(Output& output) const override;

        void setPipeline(RayTracingPipeline* pipeline) { _pipeline = pipeline; }
        RayTracingPipeline* getPipeline() { return _pipeline; }
        const RayTracingPipeline* getPipeline() const { return _pipeline; }

        void dispatch(CommandBuffer& commandBuffer) const override;

        // compile the Vulkan object, context parameter used for Device
        void compile(Context& context) override;

        virtual void release();

    public:
        virtual ~BindRayTracingPipeline();

        ref_ptr<RayTracingPipeline> _pipeline;
    };
    VSG_type_name(vsg::BindRayTracingPipeline);

} // namespace vsg
