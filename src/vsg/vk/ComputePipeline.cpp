/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>

#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/ComputePipeline.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////
//
// ComputePipeline
//
ComputePipeline::ComputePipeline()
{
}

ComputePipeline::ComputePipeline(PipelineLayout* pipelineLayout, ShaderModule* shaderModule, AllocationCallbacks* allocator) :
    _pipelineLayout(pipelineLayout),
    _shaderModule(shaderModule),
    _allocator(allocator)
{
}

ComputePipeline::~ComputePipeline()
{
}

void ComputePipeline::read(Input& input)
{
    Object::read(input);

    _pipelineLayout = input.readObject<PipelineLayout>("PipelineLayout");
    _shaderModule = input.readObject<ShaderModule>("ShaderModule");
}

void ComputePipeline::write(Output& output) const
{
    Object::write(output);

    output.writeObject("PipelineLayout", _pipelineLayout.get());
    output.writeObject("ShaderModule", _shaderModule.get());
}

void ComputePipeline::compile(Context& context)
{
    if (!_implementation)
    {
        _pipelineLayout->compile(context);
        _shaderModule->compile(context);
        _implementation = ComputePipeline::Implementation::create(context.device, _pipelineLayout, _shaderModule, _allocator);
    }
}

////////////////////////////////////////////////////////////////////////
//
// ComputePipeline::Implementation
//
ComputePipeline::Implementation::Implementation(VkPipeline pipeline, Device* device, PipelineLayout* pipelineLayout, ShaderModule* shaderModule, AllocationCallbacks* allocator) :
    _pipeline(pipeline),
    _device(device),
    _pipelineLayout(pipelineLayout),
    _shaderModule(shaderModule),
    _allocator(allocator)
{
}

ComputePipeline::Implementation::~Implementation()
{
    vkDestroyPipeline(*_device, _pipeline, _allocator);
}

ComputePipeline::Implementation::Result ComputePipeline::Implementation::create(Device* device, PipelineLayout* pipelineLayout, ShaderModule* shaderModule, AllocationCallbacks* allocator)
{
    if (!device || !pipelineLayout || !shaderModule)
    {
        return Result("Error: vsg::ComputePipeline::create(...) failed to create compute pipeline, undefined device, pipelinLayout or shaderModule.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkPipelineShaderStageCreateInfo stageInfo = {};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.module = *shaderModule;
    stageInfo.pName = shaderModule->entryPointName().c_str();
    stageInfo.pNext = nullptr;

    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = *pipelineLayout;
    pipelineInfo.stage = stageInfo;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pNext = nullptr;

    VkPipeline pipeline;
    VkResult result = vkCreateComputePipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, allocator, &pipeline);
    if (result == VK_SUCCESS)
    {
        return Result(new ComputePipeline::Implementation(pipeline, device, pipelineLayout, shaderModule, allocator));
    }
    else
    {
        return Result("Error: vsg::Pipeline::createCompute(...) failed to create VkPipeline.", result);
    }
}

////////////////////////////////////////////////////////////////////////
//
// BindComputePipeline
//
BindComputePipeline::BindComputePipeline(ComputePipeline* pipeline) :
    Inherit(0), // slot 0
    _pipeline(pipeline)
{
}

BindComputePipeline::~BindComputePipeline()
{
}

void BindComputePipeline::read(Input& input)
{
    StateCommand::read(input);

    _pipeline = input.readObject<ComputePipeline>("ComputePipeline");
}

void BindComputePipeline::write(Output& output) const
{
    StateCommand::write(output);

    output.writeObject("ComputePipeline", _pipeline.get());
}

void BindComputePipeline::dispatch(CommandBuffer& commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, *_pipeline);
    commandBuffer.setCurrentPipelineLayout(*(_pipeline->getPipelineLayout()));
}

void BindComputePipeline::compile(Context& context)
{
    if (_pipeline) _pipeline->compile(context);
}
