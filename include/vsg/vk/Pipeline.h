#pragma once

#include <vsg/vk/Swapchain.h>
#include <vsg/vk/ShaderModule.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/PipelineLayout.h>
#include <vsg/vk/CmdDraw.h>

namespace vsg
{

    class Pipeline : public Dispatch
    {
    public:
        Pipeline(Device* device, VkPipeline pipeline, AllocationCallbacks* allocator=nullptr);

        operator VkPipeline () const { return _pipeline; }

        virtual void dispatch(VkCommandBuffer commandBuffer) const
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
        }

    protected:
        virtual ~Pipeline();

        ref_ptr<Device>                 _device;
        VkPipeline                      _pipeline;
        ref_ptr<AllocationCallbacks>    _allocator;
    };


}
