#include <vsg/vk/PipelineLayout.h>

#include <iostream>

namespace vsg
{

PipelineLayout::PipelineLayout(VkPipelineLayout pipelineLayout, Device* device, const DescriptorSetLayouts& descriptorSetLayouts, AllocationCallbacks* allocator) :
    _pipelineLayout(pipelineLayout),
    _device(device),
    _allocator(allocator),
    _descriptorSetLayouts(descriptorSetLayouts)
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

PipelineLayout::Result PipelineLayout::create(Device* device, const DescriptorSetLayouts& descriptorSetLayouts, const PushConstantRanges& pushConstantRanges, VkPipelineLayoutCreateFlags flags, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Result("Error: vsg::PipelineLayout::create(...) failed to create PipelineLayout, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    std::vector<VkDescriptorSetLayout> layouts;
    for(auto dsl : descriptorSetLayouts) layouts.push_back(*dsl);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.flags = flags;
    pipelineLayoutInfo.setLayoutCount = layouts.size();
    pipelineLayoutInfo.pSetLayouts = layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size();
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();


    VkPipelineLayout pipelineLayout;
    VkResult result = vkCreatePipelineLayout(*device, &pipelineLayoutInfo, *allocator, &pipelineLayout);
    if (result == VK_SUCCESS)
    {
        return new PipelineLayout(pipelineLayout, device, descriptorSetLayouts, allocator);
    }
    else
    {
        return Result("Error: Failed to create PipelineLayout.", result);
    }
}

}