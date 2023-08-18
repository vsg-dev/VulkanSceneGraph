/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CommandBuffer
//
CommandBuffer::CommandBuffer(CommandPool* commandPool, VkCommandBuffer commandBuffer, VkCommandBufferLevel level) :
    deviceID(commandPool->getDevice()->deviceID),
    scratchMemory(ScratchMemory::create(4096)),
    _commandBuffer(commandBuffer),
    _level(level),
    _device(commandPool->getDevice()),
    _commandPool(commandPool),
    _currentPipelineLayout(VK_NULL_HANDLE),
    _currentPushConstantStageFlags(0)
{
}

CommandBuffer::~CommandBuffer()
{
    if (_commandBuffer)
    {
        _commandPool->free(this);
    }
}

void CommandBuffer::reset()
{
    _currentPipelineLayout = VK_NULL_HANDLE;
    _currentPushConstantStageFlags = 0;

    _commandPool->reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CommandBufferMap
//
CommandBufferMap::~CommandBufferMap()
{
}

void CommandBufferMap::clear()
{
    std::scoped_lock<std::mutex> lock(_mutex);
    _commandBuffers.clear();
}

void CommandBufferMap::add(int submitOder, ref_ptr<vsg::CommandBuffer> commandBuffer)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    _commandBuffers[submitOder].push_back(commandBuffer);
}

bool CommandBufferMap::empty() const
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return _commandBuffers.empty();
}

CommandBuffers CommandBufferMap::buffers() const
{
    std::scoped_lock<std::mutex> lock(_mutex);

    if (_commandBuffers.empty()) return {};
    if (_commandBuffers.size()==1) return _commandBuffers.begin()->second;

    CommandBuffers buffers;
    for(auto itr = _commandBuffers.begin(); itr != _commandBuffers.end(); ++itr)
    {
        auto& cbs = itr->second;
        buffers.insert(buffers.end(), cbs.begin(), cbs.end());
    }

    return buffers;
}
