#include <vsg/vk/BindIndexBuffer.h>
#include <vsg/vk/State.h>

namespace vsg
{

void BindIndexBuffer::pushTo(State& state)
{
    state.dirty = true;
    state.indexBufferStack.push(this);
}

void BindIndexBuffer::popFrom(State& state)
{
    state.dirty = true;
    state.indexBufferStack.pop();
}

void BindIndexBuffer::dispatch(CommandBuffer& commandBuffer) const
{
    vkCmdBindIndexBuffer(commandBuffer, *_bufferData._buffer, _bufferData._offset, _indexType);
}

}
