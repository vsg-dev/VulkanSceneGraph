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
        vkDestroyDescriptorSetLayout(*_device, _descriptorSetLayout, _allocator);
    }
}

DescriptorSetLayout::Result DescriptorSetLayout::create(Device* device, const DescriptorSetLayoutBindings& descriptorSetLayoutBindings, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Result("Error: vsg::DescriptorSetLayout::create(...) failed to create DescriptorSetLayout, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = descriptorSetLayoutBindings.size();
    layoutInfo.pBindings = descriptorSetLayoutBindings.data();

    VkDescriptorSetLayout descriptorSetLayout;
    VkResult result = vkCreateDescriptorSetLayout(*device, &layoutInfo, allocator, &descriptorSetLayout);
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
