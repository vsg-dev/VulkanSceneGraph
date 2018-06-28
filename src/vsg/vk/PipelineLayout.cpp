#include <vsg/vk/PipelineLayout.h>

#include <iostream>

namespace vsg
{

PipelineLayout::PipelineLayout(Device* device, VkPipelineLayout pipelineLayout, VkAllocationCallbacks* pAllocator) :
    _device(device),
    _pipelineLayout(pipelineLayout),
    _pAllocator(pAllocator)
{
}

PipelineLayout::PipelineLayout(Device* device, VkAllocationCallbacks* pAllocator) :
    _device(device),
    _pipelineLayout(VK_NULL_HANDLE),
    _pAllocator(pAllocator)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(*device, &pipelineLayoutInfo, pAllocator, &_pipelineLayout) != VK_SUCCESS)
    {
        std::cout<<"Failed to create VkPipelineLayout"<<std::endl;
    }
}

PipelineLayout::~PipelineLayout()
{
    if (_pipelineLayout)
    {
        std::cout<<"Calling vkDestroyPipelineLayout"<<std::endl;
        vkDestroyPipelineLayout(*_device, _pipelineLayout, _pAllocator);
    }
}

}