#pragma once

#include <vsg/vk/Pipeline.h>
#include <vsg/vk/PipelineLayout.h>
#include <vsg/vk/ShaderModule.h>

namespace vsg
{

    class ComputePipeline : public Pipeline
    {
    public:
        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        using Result = vsg::Result<ComputePipeline, VkResult, VK_SUCCESS>;

        /** Crreate a ComputePipeline.*/
        static Result create(Device* device, PipelineLayout* pipelineLayout, ShaderModule* shaderModule, AllocationCallbacks* allocator=nullptr);

    protected:
        ComputePipeline(VkPipeline pipeline, Device* device, PipelineLayout* pipelineLayout, ShaderModule* shaderModule, AllocationCallbacks* allocator);

        virtual ~ComputePipeline();

        ref_ptr<ShaderModule> _shaderModule;
    };

}
