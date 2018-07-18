#include <vsg/vk/PipelineLayout.h>

#include <iostream>

namespace vsg
{

PipelineLayout::PipelineLayout(Device* device, VkPipelineLayout pipelineLayout, AllocationCallbacks* allocator) :
    _device(device),
    _pipelineLayout(pipelineLayout),
    _allocator(allocator)
{
}

PipelineLayout::~PipelineLayout()
{
    if (_pipelineLayout)
    {
        std::cout<<"Calling vkDestroyPipelineLayout"<<std::endl;
        vkDestroyPipelineLayout(*_device, _pipelineLayout, *_allocator);
    }
}

PipelineLayout::Result PipelineLayout::create(Device* device, const VkPipelineLayoutCreateInfo& pipelineLayoutInfo, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Result("Error: vsg::PipelineLayout::create(...) failed to create PipelineLayout, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkPipelineLayout pipelineLayout;
    VkResult result = vkCreatePipelineLayout(*device, &pipelineLayoutInfo, *allocator, &pipelineLayout);
    if (result == VK_SUCCESS)
    {
        return new PipelineLayout(device, pipelineLayout, allocator);
    }
    else
    {
        return Result("Error: Failed to create PipelineLayout.", result);
    }
}

PipelineLayout::Result PipelineLayout::create(Device* device, AllocationCallbacks* allocator)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    return create(device, pipelineLayoutInfo, allocator);
}

}