#include <vsg/vk/DescriptorSetLayout.h>

#include <iostream>

namespace vsg
{

DescriptorSetLayout::DescriptorSetLayout(Device* device, VkDescriptorSetLayout descriptorSetLayout, AllocationCallbacks* allocator) :
    _device(device),
    _descriptorSetLayout(descriptorSetLayout),
    _allocator(allocator)
{
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    std::cout<<"Calling vkDestroyDescriptorSetLayout "<<_descriptorSetLayout<<std::endl;
    if (_descriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(*_device, _descriptorSetLayout, *_allocator);
    }
}

DescriptorSetLayout::Result DescriptorSetLayout::create(Device* device, const VkDescriptorSetLayoutCreateInfo& descriptorSetLayoutInfo, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Result("Error: vsg::DescriptorSetLayout::create(...) failed to create DescriptorSetLayout, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkDescriptorSetLayout descriptorSetLayout;
    VkResult result = vkCreateDescriptorSetLayout(*device, &descriptorSetLayoutInfo, *allocator, &descriptorSetLayout);
    if (result == VK_SUCCESS)
    {
        return new DescriptorSetLayout(device, descriptorSetLayout, allocator);
    }
    else
    {
        return Result("Error: Failed to create DescriptorSetLayout.", result);
    }
}

}