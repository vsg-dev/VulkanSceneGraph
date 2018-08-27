#include <vsg/vk/BindVertexBuffers.h>
#include <vsg/vk/State.h>

namespace vsg
{

void BindVertexBuffers::pushTo(State& state)
{
    state.dirty = true;
    state.vertexBuffersStack.push(this);
}

void BindVertexBuffers::popFrom(State& state)
{
    state.dirty = true;
    state.vertexBuffersStack.pop();
}

void BindVertexBuffers::dispatch(CommandBuffer& commandBuffer) const
{
    vkCmdBindVertexBuffers(commandBuffer, _firstBinding, _buffers.size(), _vkBuffers.data(), _offsets.data());
}

}
