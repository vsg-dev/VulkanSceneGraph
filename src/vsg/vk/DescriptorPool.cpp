/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/io/Options.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/Context.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorSetLayout.h>

#include <iostream>

using namespace vsg;

static size_t s_num_DescriptorPool_allocatoed = 0;
static size_t s_num_DescriptorPool_deallocator = 0;

DescriptorPool::DescriptorPool(Device* device, uint32_t maxSets, const DescriptorPoolSizes& descriptorPoolSizes) :
    _device(device),
    _availableDescriptorSet(maxSets),
    _availableDescriptorPoolSizes(descriptorPoolSizes)
{
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    poolInfo.pPoolSizes = descriptorPoolSizes.data();
    poolInfo.maxSets = maxSets;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // will we need VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT later?
    poolInfo.pNext = nullptr;

    if (VkResult result = vkCreateDescriptorPool(*device, &poolInfo, _device->getAllocationCallbacks(), &_descriptorPool); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create DescriptorPool.", result};
    }

    ++s_num_DescriptorPool_allocatoed;
    std::cout<<"DescriptorPool::DescriptorPool(maxSets = "<<maxSets<<") "<<s_num_DescriptorPool_allocatoed<<", "<<s_num_DescriptorPool_deallocator<<std::endl;
}

DescriptorPool::~DescriptorPool()
{
    if (_descriptorPool)
    {
        vkDestroyDescriptorPool(*_device, _descriptorPool, _device->getAllocationCallbacks());
    }

    std::cout<<"DescriptorPool::~DescriptorPool() _reclingList.size() = "<<_reclingList.size()<<std::endl;
    ++s_num_DescriptorPool_deallocator;

    std::cout<<"DescriptorPool::~DescriptorPool() "<<s_num_DescriptorPool_allocatoed<<", "<<s_num_DescriptorPool_deallocator<<std::endl;
}

ref_ptr<DescriptorSet_Implementation> DescriptorPool::allocateDescriptorSet(DescriptorSetLayout* descriptorSetLayout)
{
    if (_availableDescriptorSet==0)
    {
        std::cout<<"DescriptorPool::allocateDescriptorSet() "<<this<<" nothing available _availableDescriptorSet = "<<_availableDescriptorSet<<std::endl;
        return {};
    }

    DescriptorPoolSizes descriptorPoolSizes;
    descriptorSetLayout->getDescriptorPoolSizes(descriptorPoolSizes);

    for(auto itr = _reclingList.begin(); itr != _reclingList.end(); ++itr)
    {
        if (vsg::compare_value_container(descriptorPoolSizes, (*itr)->_descriptorPoolSizes)==0)
        {
            auto dsi = *itr;
            dsi->_descriptorPool = this;
            _reclingList.erase(itr);
            --_availableDescriptorSet;
            std::cout<<"    returning recyled dsi "<<dsi<<std::endl;
            return dsi;
        }
    }

    size_t matches = 0;
    for(auto& [type, descriptorCount] : descriptorPoolSizes)
    {
        for(auto& [availableType, availableCount] : _availableDescriptorPoolSizes)
        {
            if (availableType == type)
            {
                if (availableCount >= descriptorCount) ++matches;
            }
        }
    }

    if (matches < descriptorPoolSizes.size())
    {
        std::cout<<"DescriptorPool::allocateDescriptorSet("<<descriptorSetLayout<<") not enough space,"<<std::endl;
        return {};
    }

    for(auto& [type, descriptorCount] : descriptorPoolSizes)
    {
        for(auto& [availableType, availableCount] : _availableDescriptorPoolSizes)
        {
            if (availableType == type)
            {
                availableCount -= descriptorCount;
            }
        }
    }

    --_availableDescriptorSet;

    auto dsi = DescriptorSet_Implementation::create(this, descriptorSetLayout);
    std::cout<<"DescriptorPool::allocateDescriptorSet("<<descriptorSetLayout<<") dsi = "<<dsi<<std::endl;
    return dsi;
}

void DescriptorPool::freeDescriptorSet(ref_ptr<DescriptorSet_Implementation> dsi)
{
    std::cout<<"DescriptorPool::freeDescriptorSet("<<dsi<<")"<<std::endl;

    _reclingList.push_back(dsi);
    ++_availableDescriptorSet;

    dsi->_descriptorPool = {};
}

DescriptorSet_Implementation::DescriptorSet_Implementation(DescriptorPool* descriptorPool, DescriptorSetLayout* descriptorSetLayout) :
    _descriptorPool(descriptorPool)
{
    auto device = descriptorPool->getDevice();

    std::cout<<"DescriptorSet_Implementation::DescriptorSet_Implementation("<<descriptorPool<<", "<<descriptorSetLayout<<") "<<this<<std::endl;
    _descriptorPoolSizes.clear();
    descriptorSetLayout->getDescriptorPoolSizes(_descriptorPoolSizes);

    VkDescriptorSetLayout vkdescriptorSetLayout = descriptorSetLayout->vk(device->deviceID);

    VkDescriptorSetAllocateInfo descriptSetAllocateInfo = {};
    descriptSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptSetAllocateInfo.descriptorPool = *descriptorPool;
    descriptSetAllocateInfo.descriptorSetCount = 1;
    descriptSetAllocateInfo.pSetLayouts = &vkdescriptorSetLayout;

    if (VkResult result = vkAllocateDescriptorSets(*device, &descriptSetAllocateInfo, &_descriptorSet); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create DescriptorSet.", result};
    }
}

DescriptorSet_Implementation::~DescriptorSet_Implementation()
{
    std::cout<<"DescriptorSet_Implementation::~DescriptorSet_Implementation() "<<this<<" "<<_descriptorPool<<std::endl;

    for(auto& [type, descriptorCount] : _descriptorPoolSizes)
    {
        std::cout<<"    type = "<<type<<", count = "<<descriptorCount<<std::endl;
    }

    if (_descriptorPool &&_descriptorSet)
    {
#if USE_MUTEX
        std::scoped_lock<std::mutex> lock(_descriptorPool->getMutex());
#endif
        auto device = _descriptorPool->getDevice();

        // VkPhysicalDeviceVulkanSC10Properties.recycleDescriptorSetMemory
        vkFreeDescriptorSets(*device, *_descriptorPool, 1, &_descriptorSet);
    }
}

void DescriptorSet_Implementation::assign(Context& context, const Descriptors& descriptors)
{
    // should we doing anything about previous _descriptor that may have been assigned?
    _descriptors = descriptors;

    if (_descriptors.empty()) return;

    VkWriteDescriptorSet* descriptorWrites = context.scratchMemory->allocate<VkWriteDescriptorSet>(_descriptors.size());

    for (size_t i = 0; i < _descriptors.size(); ++i)
    {
        descriptors[i]->assignTo(context, descriptorWrites[i]);
        descriptorWrites[i].dstSet = _descriptorSet;
    }

    auto device = _descriptorPool->getDevice();
    vkUpdateDescriptorSets(*device, static_cast<uint32_t>(descriptors.size()), descriptorWrites, 0, nullptr);

    // clean up scratch memory so it can be reused.
    context.scratchMemory->release();
}

void vsg::recyle(ref_ptr<DescriptorSet_Implementation>& dsi)
{
    if (dsi)
    {
        if (dsi->_descriptorPool) dsi->_descriptorPool->freeDescriptorSet(dsi);
        dsi = {};
    }
}
