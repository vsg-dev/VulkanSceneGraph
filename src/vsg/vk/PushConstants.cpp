#include <vsg/vk/PushConstants.h>
#include <vsg/vk/State.h>

namespace vsg
{

PushConstants::PushConstants(VkShaderStageFlags stageFlags, uint32_t offset, Data* data):
    _stageFlags(stageFlags),
    _offset(offset),
    _data(data)
{
}

PushConstants::~PushConstants()
{
}

void PushConstants::pushTo(State& state)
{
    state.pushConstantsStack.push(this);
}

void PushConstants::popFrom(State& state)
{
    state.pushConstantsStack.pop();
}

void PushConstants::dispatch(CommandBuffer& commandBuffer) const
{
    const PipelineLayout* pipelineLayout = commandBuffer.getCurrentPipelineLayout();
    if (pipelineLayout)
    {

        vkCmdPushConstants(commandBuffer, *pipelineLayout, _stageFlags, _offset, _data->dataSize(), _data->dataPointer());
    }
}

}
