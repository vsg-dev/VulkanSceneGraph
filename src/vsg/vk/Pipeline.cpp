#include <vsg/vk/Pipeline.h>
#include <vsg/vk/CommandBuffer.h>

namespace vsg
{

////////////////////////////////////////////////////////////////////////
//
// Pipeline
//
Pipeline::Pipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint, Device* device, PipelineLayout* pipelineLayout, AllocationCallbacks* allocator) :
    _pipeline(pipeline),
    _bindPoint(bindPoint),
    _device(device),
    _pipelineLayout(pipelineLayout),
    _allocator(allocator)
{
}

Pipeline::~Pipeline()
{
    if (_pipeline)
    {
        vkDestroyPipeline(*_device, _pipeline, _allocator);
    }
}

void Pipeline::dispatch(CommandBuffer& commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, _bindPoint, _pipeline);
    commandBuffer.setCurrentPipeline(this);
}

}
