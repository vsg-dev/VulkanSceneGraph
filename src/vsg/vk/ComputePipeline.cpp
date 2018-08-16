#include <vsg/vk/ComputePipeline.h>

namespace vsg
{

ComputePipeline::ComputePipeline(VkPipeline pipeline, Device* device, PipelineLayout* pipelineLayout, ShaderModule* shaderModule, AllocationCallbacks* allocator):
    Pipeline(pipeline, VK_PIPELINE_BIND_POINT_COMPUTE, device, allocator),
    _pipelineLayout(pipelineLayout),
    _shaderModule(shaderModule)
{
}

ComputePipeline::~ComputePipeline()
{
}

ComputePipeline::Result ComputePipeline::create(Device* device, PipelineLayout* pipelineLayout, ShaderModule* shaderModule, AllocationCallbacks* allocator)
{
    if (!device || !pipelineLayout || !shaderModule)
    {
        return ComputePipeline::Result("Error: vsg::ComputePipeline::create(...) failed to create compute pipeline, undefined device, pipelinLayout or shaderModule.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkPipelineShaderStageCreateInfo stageInfo = {};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.module = *shaderModule;
    stageInfo.pName = shaderModule->getEntryPointName().c_str();

    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = *pipelineLayout;
    pipelineInfo.stage = stageInfo;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline pipeline;
    VkResult result = vkCreateComputePipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, allocator, &pipeline );
    if (result == VK_SUCCESS)
    {
        return new ComputePipeline(pipeline, device, pipelineLayout, shaderModule, allocator);
    }
    else
    {
        return ComputePipeline::Result("Error: vsg::Pipeline::createCompute(...) failed to create VkPipeline.", result);
    }
}

}
