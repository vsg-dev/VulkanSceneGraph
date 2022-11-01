/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/DescriptorPool.h>

using namespace vsg;

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
}

DescriptorPool::~DescriptorPool()
{
    if (_descriptorPool)
    {
        vkDestroyDescriptorPool(*_device, _descriptorPool, _device->getAllocationCallbacks());
    }
}

ref_ptr<DescriptorSet::Implementation> DescriptorPool::allocateDescriptorSet(DescriptorSetLayout* descriptorSetLayout)
{
    std::scoped_lock<std::mutex> lock(mutex);

    if (_availableDescriptorSet == 0)
    {
        return {};
    }

    DescriptorPoolSizes descriptorPoolSizes;
    descriptorSetLayout->getDescriptorPoolSizes(descriptorPoolSizes);

    for (auto itr = _reclingList.begin(); itr != _reclingList.end(); ++itr)
    {
        if (vsg::compare_value_container(descriptorPoolSizes, (*itr)->_descriptorPoolSizes) == 0)
        {
            auto dsi = *itr;
            dsi->_descriptorPool = this;
            _reclingList.erase(itr);
            --_availableDescriptorSet;
            // debug("DescriptorPool::allocateDescriptorSet(..) reusing ", dsi)   ;
            return dsi;
        }
    }

    if (_availableDescriptorSet == _reclingList.size())
    {
        //debug("The only available vkDescriptoSets associated with DescriptorPool are in the recyclingList, but none are compatible.");
        return {};
    }

    size_t matches = 0;
    for (auto& [type, descriptorCount] : descriptorPoolSizes)
    {
        for (auto& [availableType, availableCount] : _availableDescriptorPoolSizes)
        {
            if (availableType == type)
            {
                if (availableCount >= descriptorCount) ++matches;
            }
        }
    }

    if (matches < descriptorPoolSizes.size())
    {
        return {};
    }

    for (auto& [type, descriptorCount] : descriptorPoolSizes)
    {
        for (auto& [availableType, availableCount] : _availableDescriptorPoolSizes)
        {
            if (availableType == type)
            {
                availableCount -= descriptorCount;
            }
        }
    }

    --_availableDescriptorSet;

    auto dsi = DescriptorSet::Implementation::create(this, descriptorSetLayout);
    //debug("DescriptorPool::allocateDescriptorSet(..) allocated ", dsi);
    return dsi;
}

void DescriptorPool::freeDescriptorSet(ref_ptr<DescriptorSet::Implementation> dsi)
{
    {
        std::scoped_lock<std::mutex> lock(mutex);
        _reclingList.push_back(dsi);
        ++_availableDescriptorSet;
    }

    dsi->_descriptorPool = {};
}

bool DescriptorPool::getAvailablity(uint32_t& maxSets, DescriptorPoolSizes& descriptorPoolSizes) const
{
    std::scoped_lock<std::mutex> lock(mutex);

    if (_availableDescriptorSet == 0) return false;

    maxSets += _availableDescriptorSet;

    for (auto& [availableType, availableCount] : _availableDescriptorPoolSizes)
    {
        auto itr = descriptorPoolSizes.begin();
        for (; itr != descriptorPoolSizes.end(); ++itr)
        {
            if (itr->type == availableType)
            {
                itr->descriptorCount += availableCount;
                break;
            }
        }
        if (itr != descriptorPoolSizes.end())
        {
            descriptorPoolSizes.push_back(VkDescriptorPoolSize{availableType, availableCount});
        }
    }

    return true;
}
