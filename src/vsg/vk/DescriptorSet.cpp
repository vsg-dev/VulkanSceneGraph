#include <vsg/vk/DescriptorSet.h>

#include <iostream>

namespace vsg
{

DescriptorSet::DescriptorSet(Device* device, DescriptorPool* descriptorPool, DescriptorSetLayout* descriptorSetLayout, VkDescriptorSet descriptorSet) :
    _device(device),
    _descriptorPool(descriptorPool),
    _descriptorSetLayout(descriptorSetLayout),
    _descriptorSet(descriptorSet)
{
}

DescriptorSet::~DescriptorSet()
{
    std::cout<<"Calling vkFreeDescriptorSets "<<_descriptorSet<<std::endl;
    if (_descriptorSet)
    {
        vkFreeDescriptorSets(*_device, *_descriptorPool, 1, &_descriptorSet);
    }
}

DescriptorSet::Result DescriptorSet::create(Device* device, DescriptorPool* descriptorPool, DescriptorSetLayout* descriptorSetLayout)
{
    if (!device || !descriptorPool || !descriptorSetLayout)
    {
        return Result("Error: vsg::DescriptorPool::create(...) failed to create DescriptorPool, undefined Device, DescriptorPool or DescriptorSetLayout.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    VkDescriptorSetLayout descriptorSetLayouts[] = {*descriptorSetLayout};

    VkDescriptorSetAllocateInfo descriptSetAllocateInfo = {};
    descriptSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptSetAllocateInfo.descriptorPool = *descriptorPool;
    descriptSetAllocateInfo.descriptorSetCount = 1;
    descriptSetAllocateInfo.pSetLayouts = descriptorSetLayouts;

    VkDescriptorSet descriptorSet;
    VkResult result = vkAllocateDescriptorSets(*device, &descriptSetAllocateInfo, &descriptorSet);
    if (result == VK_SUCCESS)
    {
        std::cout<<"Creating DescriptorSet "<<descriptorSet<<std::endl;
        return new DescriptorSet(device, descriptorPool, descriptorSetLayout, descriptorSet);
    }
    else
    {
        return Result("Error: Failed to create DescriptorPool.", result);
    }
}

}