#include <vsg/vk/DescriptorSet.h>

#include <iostream>

namespace vsg
{

DescriptorSet::DescriptorSet(VkDescriptorSet descriptorSet, Device* device, DescriptorPool* descriptorPool, DescriptorSetLayout* descriptorSetLayout, const Descriptors& descriptors) :
    _descriptorSet(descriptorSet),
    _device(device),
    _descriptorPool(descriptorPool),
    _descriptorSetLayout(descriptorSetLayout)
{
    assign(descriptors);
}

DescriptorSet::~DescriptorSet()
{
    std::cout<<"Calling vkFreeDescriptorSets "<<_descriptorSet<<std::endl;
    if (_descriptorSet)
    {
        vkFreeDescriptorSets(*_device, *_descriptorPool, 1, &_descriptorSet);
    }
}

DescriptorSet::Result DescriptorSet::create(Device* device, DescriptorPool* descriptorPool, DescriptorSetLayout* descriptorSetLayout, const Descriptors& descriptors)
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
        return new DescriptorSet(descriptorSet, device, descriptorPool, descriptorSetLayout, descriptors);
    }
    else
    {
        return Result("Error: Failed to create DescriptorPool.", result);
    }
}

void DescriptorSet::assign(const Descriptors& descriptors)
{
    // should we doing anything about previous _descriptor that may have been assigned?
    _descriptors = descriptors;

    std::vector<VkWriteDescriptorSet> descriptorWrites(_descriptors.size());
    for (size_t i=0; i<_descriptors.size(); ++i)
    {
        _descriptors[i]->assignTo(descriptorWrites[i], _descriptorSet);
    }

    vkUpdateDescriptorSets(*_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

}