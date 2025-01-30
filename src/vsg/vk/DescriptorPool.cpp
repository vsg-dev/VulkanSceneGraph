/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/compare.h>
#include <vsg/io/Logger.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/vk/Context.h>
#include <vsg/vk/DescriptorPool.h>

using namespace vsg;

DescriptorPool::DescriptorPool(Device* device, uint32_t in_maxSets, const DescriptorPoolSizes& in_descriptorPoolSizes) :
    maxSets(in_maxSets),
    descriptorPoolSizes(in_descriptorPoolSizes),
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

    vsg::debug("DescriptorPool::DescriptorPool() ", this, ", maxSets = ", maxSets, " {");
    for (auto& dps : descriptorPoolSizes)
    {
        vsg::debug("   { ", dps.type, ", ", dps.descriptorCount, " }");
    }
    vsg::debug("}");
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

    for (auto itr = _recyclingList.begin(); itr != _recyclingList.end(); ++itr)
    {
        auto dsi = *itr;
        if (dsi->_descriptorSetLayout.get() == descriptorSetLayout || compare_value_container(dsi->_descriptorSetLayout->bindings, descriptorSetLayout->bindings) == 0)
        {
            // swap ownership so that DescriptorSet::Implementation now "has a" reference to this DescriptorPool
            dsi->_descriptorPool = this;
            _recyclingList.erase(itr);
            --_availableDescriptorSet;
            return dsi;
        }
    }

    if (_availableDescriptorSet == _recyclingList.size())
    {
        vsg::debug("The only available vkDescriptorSets associated with DescriptorPool are in the recyclingList, but none are compatible.");
        return {};
    }

    DescriptorPoolSizes requiredDescriptorPoolSizes;
    descriptorSetLayout->getDescriptorPoolSizes(requiredDescriptorPoolSizes);

    auto newDescriptorPoolSizes = _availableDescriptorPoolSizes;
    for (auto& [type, descriptorCount] : requiredDescriptorPoolSizes)
    {
        uint32_t foundDescriptorCount = 0;
        for (auto& [availableType, availableCount] : newDescriptorPoolSizes)
        {
            if (availableType == type)
            {
                uint32_t descriptorsToConsume = descriptorCount - foundDescriptorCount;
                if (descriptorsToConsume > availableCount)
                    descriptorsToConsume = availableCount;
                foundDescriptorCount += descriptorsToConsume;
                availableCount -= descriptorsToConsume;
            }
        }
        if (foundDescriptorCount < descriptorCount)
            return {};
    }

    _availableDescriptorPoolSizes.swap(newDescriptorPoolSizes);
    --_availableDescriptorSet;

    auto dsi = DescriptorSet::Implementation::create(this, descriptorSetLayout);
    vsg::debug("DescriptorPool::allocateDescriptorSet(..) allocated new ", dsi);
    return dsi;
}

void DescriptorPool::freeDescriptorSet(ref_ptr<DescriptorSet::Implementation> dsi)
{
    {
        // swap ownership so that DescriptorSet::Implementation' reference is reset to null and while this DescriptorPool takes a reference to it.
        // aquire lock within local scope so that subsequent dsi->_descriptorPool = {} call doesn't unref and (possibly) delete this DescriptorPool while lock still held.
        std::scoped_lock<std::mutex> lock(mutex);
        _recyclingList.push_back(dsi);
        ++_availableDescriptorSet;
    }
    dsi->_descriptorPool = {};
}

bool DescriptorPool::available(uint32_t& numSets, DescriptorPoolSizes& availableDescriptorPoolSizes) const
{
    std::scoped_lock<std::mutex> lock(mutex);

    if (_availableDescriptorSet == 0) return false;

    numSets += _availableDescriptorSet;

    for (auto& dps : descriptorPoolSizes)
    {
        if (dps.descriptorCount > 0)
        {
            // increment any entries that are already in the descriptorPoolSizes vector
            auto itr = std::find_if(availableDescriptorPoolSizes.begin(), availableDescriptorPoolSizes.end(), [&dps](const VkDescriptorPoolSize& value) { return value.type == dps.type; });
            if (itr != availableDescriptorPoolSizes.end())
                itr->descriptorCount += dps.descriptorCount;
            else
                availableDescriptorPoolSizes.push_back(VkDescriptorPoolSize{dps.type, dps.descriptorCount});
        }
    }

    for (const auto& dsi : _recyclingList)
    {
        if (dsi->_descriptorSetLayout)
        {
            for (auto& binding : dsi->_descriptorSetLayout->bindings)
            {
                // increment any entries that are already in the descriptorPoolSizes vector
                auto itr = std::find_if(availableDescriptorPoolSizes.begin(), availableDescriptorPoolSizes.end(), [&binding](const VkDescriptorPoolSize& value) { return value.type == binding.descriptorType; });
                if (itr != availableDescriptorPoolSizes.end())
                    itr->descriptorCount += binding.descriptorCount;
                else
                    availableDescriptorPoolSizes.push_back(VkDescriptorPoolSize{binding.descriptorType, binding.descriptorCount});
            }
        }
    }

    return true;
}

bool DescriptorPool::used(uint32_t& numSets, DescriptorPoolSizes& usedDescriptorPoolSizes) const
{
    if (maxSets == _availableDescriptorSet) return false;

    numSets += maxSets - _availableDescriptorSet;

    for (auto& dps : descriptorPoolSizes)
    {
        auto itr = std::find_if(_availableDescriptorPoolSizes.begin(), _availableDescriptorPoolSizes.end(), [&dps](const VkDescriptorPoolSize& value) { return value.type == dps.type; });
        if (itr != _availableDescriptorPoolSizes.end())
        {
            uint32_t usedDescriptorCount = dps.descriptorCount - itr->descriptorCount;
            auto used_itr = std::find_if(usedDescriptorPoolSizes.begin(), usedDescriptorPoolSizes.end(), [&dps](const VkDescriptorPoolSize& value) { return value.type == dps.type; });
            if (used_itr != usedDescriptorPoolSizes.end())
                used_itr += usedDescriptorCount;
            else
                usedDescriptorPoolSizes.push_back(VkDescriptorPoolSize{dps.type, usedDescriptorCount});
        }
    }
    return true;
}

void DescriptorPool::report(std::ostream& out, indentation indent) const
{
    out << indent << "DescriptorPool " << this << " {" << std::endl;
    indent += 4;

    out << indent << "maxSets = " << maxSets << std::endl;
    out << indent << "descriptorPoolSizes = " << descriptorPoolSizes.size() << " {" << std::endl;
    indent += 4;
    for (const auto& dps : descriptorPoolSizes)
    {
        out << indent << "VkDescriptorPoolSize { " << dps.type << ", " << dps.descriptorCount << " }" << std::endl;
    }
    indent -= 4;
    out << indent << "}" << std::endl;

    out << indent << "_availableDescriptorSet = " << _availableDescriptorSet << std::endl;
    out << indent << "_availableDescriptorPoolSizes = " << _availableDescriptorPoolSizes.size() << " {" << std::endl;
    indent += 4;
    for (const auto& dps : _availableDescriptorPoolSizes)
    {
        out << indent << "VkDescriptorPoolSize { " << dps.type << ", " << dps.descriptorCount << " }" << std::endl;
    }
    indent -= 4;
    out << indent << "}" << std::endl;

    out << indent << "_recyclingList " << _recyclingList.size() << " {" << std::endl;
    indent += 4;
    for (const auto& dsi : _recyclingList)
    {
        out << indent << "DescriptorSet::Implementation " << dsi << ", descriptorSetLayout =  " << dsi->_descriptorSetLayout << " {" << std::endl;
        indent += 4;
        if (dsi->_descriptorSetLayout)
        {
            for (const auto& binding : dsi->_descriptorSetLayout->bindings)
            {
                out << indent << "VkDescriptorSetLayoutBinding { " << binding.binding << ", " << binding.descriptorType << ", " << binding.stageFlags << ", " << binding.pImmutableSamplers << " }" << std::endl;
            }
        }
        indent -= 4;
        out << indent << " }" << std::endl;
    }
    indent -= 4;
    out << indent << "}" << std::endl;

    indent -= 4;
    out << indent << "}" << std::endl;
}
