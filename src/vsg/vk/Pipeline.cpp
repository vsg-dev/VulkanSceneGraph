#include <vsg/vk/Pipeline.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/State.h>

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

BindPipeline::BindPipeline(Pipeline* pipeline):
    _pipeline(pipeline)
{
}

BindPipeline::~BindPipeline()
{
}

void BindPipeline::pushTo(State& state)
{
    state.dirty = true;
    state.pipelineStack.push(this);
}

void BindPipeline::popFrom(State& state)
{
    state.dirty = true;
    state.pipelineStack.pop();
}

void BindPipeline::dispatch(CommandBuffer& commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, _pipeline->getBindPoint(), *_pipeline);
    commandBuffer.setCurrentPipeline(_pipeline);
}
}
