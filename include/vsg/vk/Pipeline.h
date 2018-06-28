#pragma once

#include <vsg/vk/Swapchain.h>
#include <vsg/vk/ShaderModule.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/PipelineLayout.h>

namespace vsg
{

    class Pipeline : public vsg::Object
    {
    public:
        Pipeline(Device* device, VkPipeline pipeline, VkAllocationCallbacks* pAllocator=nullptr);

        operator VkPipeline () const { return _pipeline; }

    protected:
        virtual ~Pipeline();

        ref_ptr<Device>         _device;
        VkPipeline              _pipeline;
        VkAllocationCallbacks*  _pAllocator;
    };

    extern ref_ptr<Pipeline> createGraphicsPipeline(Device* device, Swapchain* swapchain, RenderPass* renderPass, PipelineLayout* pipelineLayout, ShaderModule* vert, ShaderModule* frag, VkAllocationCallbacks* pAllocator=nullptr);

}
