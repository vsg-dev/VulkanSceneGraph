/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Pipeline.h>
#include <vsg/vk/State.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////
//
// Pipeline
//
Pipeline::Pipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint, Device* device, PipelineLayout::Implementation* pipelineLayout, AllocationCallbacks* allocator) :
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

BindPipeline::BindPipeline(Pipeline* pipeline) :
    _pipeline(pipeline)
{
}

BindPipeline::~BindPipeline()
{
}

void BindPipeline::pushTo(State& state) const
{
    state.dirty = true;
    state.pipelineStack.push(this);
}

void BindPipeline::popFrom(State& state) const
{
    state.dirty = true;
    state.pipelineStack.pop();
}

void BindPipeline::dispatch(CommandBuffer& commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, _pipeline->getBindPoint(), *_pipeline);
    commandBuffer.setCurrentPipeline(_pipeline);
}
