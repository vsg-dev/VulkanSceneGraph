/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/vk/DescriptorPools.h>

#include <iostream>

using namespace vsg;

DescriptorPools::DescriptorPools(ref_ptr<Device> in_device) :
    device(in_device)
{
}

DescriptorPools::~DescriptorPools()
{
}

void DescriptorPools::getDescriptorPoolSizesToUse(uint32_t& maxSets, DescriptorPoolSizes& descriptorPoolSizes)
{
    if (reserve_count > 0)
    {
        if (minimum_maxSets > reserve_maxSets)
        {
            for (auto& dps : reserve_descriptorPoolSizes)
            {
                dps.descriptorCount = static_cast<uint32_t>(std::ceil(static_cast<double>(dps.descriptorCount) * static_cast<double>(minimum_maxSets) / static_cast<double>(reserve_maxSets)));
            }
            reserve_maxSets = minimum_maxSets;
        }

        if (minimum_maxSets > maxSets)
        {
            maxSets = minimum_maxSets;
        }

        for (auto& [type, descriptorCount] : reserve_descriptorPoolSizes)
        {
            auto itr = descriptorPoolSizes.begin();
            for (; itr != descriptorPoolSizes.end(); ++itr)
            {
                if (itr->type == type)
                {
                    if (descriptorCount > itr->descriptorCount)
                        itr->descriptorCount = descriptorCount;
                    break;
                }
            }
            if (itr == descriptorPoolSizes.end())
            {
                descriptorPoolSizes.push_back(VkDescriptorPoolSize{type, descriptorCount});
            }
        }
    }

    minimum_maxSets = std::min(maximum_maxSets, static_cast<uint32_t>(static_cast<double>(maxSets) * scale_maxSets));

    reserve_count = 0;
    reserve_maxSets = 0;
    reserve_descriptorPoolSizes.clear();
}

void DescriptorPools::reserve(const ResourceRequirements& requirements)
{
    auto maxSets = requirements.computeNumDescriptorSets();
    auto descriptorPoolSizes = requirements.computeDescriptorPoolSizes();

    // update the variables tracing all reserve calls.
    ++reserve_count;
    reserve_maxSets += maxSets;
    for (auto& dps : descriptorPoolSizes)
    {
        auto itr = std::find_if(reserve_descriptorPoolSizes.begin(), reserve_descriptorPoolSizes.end(), [&dps](const VkDescriptorPoolSize& value) { return value.type == dps.type; });
        if (itr != reserve_descriptorPoolSizes.end())
            itr->descriptorCount += dps.descriptorCount;
        else
            reserve_descriptorPoolSizes.push_back(dps);
    }

    // compute the total available resources
    uint32_t available_maxSets = 0;
    DescriptorPoolSizes available_descriptorPoolSizes;
    for (auto& descriptorPool : descriptorPools)
    {
        descriptorPool->available(available_maxSets, available_descriptorPoolSizes);
    }

    // compute the additional required resources
    auto required_maxSets = maxSets;
    if (available_maxSets < required_maxSets)
        required_maxSets -= available_maxSets;
    else
        required_maxSets = 0;

    DescriptorPoolSizes required_descriptorPoolSizes;
    for (const auto& [type, descriptorCount] : descriptorPoolSizes)
    {
        uint32_t adjustedDescriptorCount = descriptorCount;
        for (auto itr = available_descriptorPoolSizes.begin(); itr != available_descriptorPoolSizes.end(); ++itr)
        {
            if (itr->type == type)
            {
                if (itr->descriptorCount < adjustedDescriptorCount)
                    adjustedDescriptorCount -= itr->descriptorCount;
                else
                {
                    adjustedDescriptorCount = 0;
                    break;
                }
            }
        }
        if (adjustedDescriptorCount > 0)
            required_descriptorPoolSizes.push_back(VkDescriptorPoolSize{type, adjustedDescriptorCount});
    }

    // check if all the requirements have been met by exisiting availability
    if (required_maxSets == 0 && required_descriptorPoolSizes.empty())
    {
        vsg::debug("DescriptorPools::reserve(const ResourceRequirements& requirements) enought resource in existing DescriptorPools");
        return;
    }

    // not enough descriptor resources available so allocator new DescriptorPool.
    getDescriptorPoolSizesToUse(required_maxSets, required_descriptorPoolSizes);
    descriptorPools.push_back(vsg::DescriptorPool::create(device, required_maxSets, required_descriptorPoolSizes));
}

ref_ptr<DescriptorSet::Implementation> DescriptorPools::allocateDescriptorSet(DescriptorSetLayout* descriptorSetLayout)
{
    for (auto itr = descriptorPools.rbegin(); itr != descriptorPools.rend(); ++itr)
    {
        auto dsi = (*itr)->allocateDescriptorSet(descriptorSetLayout);
        if (dsi) return dsi;
    }

    DescriptorPoolSizes descriptorPoolSizes;
    descriptorSetLayout->getDescriptorPoolSizes(descriptorPoolSizes);

    uint32_t maxSets = 1;
    getDescriptorPoolSizesToUse(maxSets, descriptorPoolSizes);

    auto descriptorPool = vsg::DescriptorPool::create(device, maxSets, descriptorPoolSizes);
    auto dsi = descriptorPool->allocateDescriptorSet(descriptorSetLayout);

    descriptorPools.push_back(descriptorPool);
    return dsi;
}

void DescriptorPools::report(std::ostream& out, indentation indent) const
{
    auto print = [&out, &indent](const std::string_view& name, uint32_t numSets, const DescriptorPoolSizes& descriptorPoolSizes) {
        out << indent << name << " {" << std::endl;
        indent += 4;
        out << indent << "numSets " << numSets << std::endl;
        out << indent << "descriptorPoolSizes " << descriptorPoolSizes.size() << " {" << std::endl;
        indent += 4;
        for (const auto& dps : descriptorPoolSizes)
        {
            out << indent << "VkDescriptorPoolSize{ " << dps.type << ", " << dps.descriptorCount << " }" << std::endl;
        }
        indent -= 4;
        out << indent << "}" << std::endl;

        indent -= 4;
        out << indent << "}" << std::endl;
    };

    out << "DescriptorPools::report(..) " << this << " {" << std::endl;
    indent += 4;

#if 1
    out << indent << "descriptorPools " << descriptorPools.size() << std::endl;
#else
    out << indent << "descriptorPools " << descriptorPools.size() << " {" << std::endl;
    indent += 4;
    for (auto& dp : descriptorPools)
    {
        dp->report(out, indent);
    }
    indent -= 4;
    out << indent << "}" << std::endl;
#endif

    uint32_t numSets = 0;
    DescriptorPoolSizes descriptorPoolSizes;

    allocated(numSets, descriptorPoolSizes);
    print("DescriptorPools::allocated()", numSets, descriptorPoolSizes);

    numSets = 0;
    descriptorPoolSizes.clear();
    used(numSets, descriptorPoolSizes);
    print("DescriptorPools::used()", numSets, descriptorPoolSizes);

    numSets = 0;
    descriptorPoolSizes.clear();
    available(numSets, descriptorPoolSizes);
    print("DescriptorPools::available()", numSets, descriptorPoolSizes);

    indent -= 4;
    out << indent << "}" << std::endl;
}

bool DescriptorPools::available(uint32_t& numSets, DescriptorPoolSizes& availableDescriptorPoolSizes) const
{
    bool result = false;
    for (auto& dp : descriptorPools)
    {
        result = dp->available(numSets, availableDescriptorPoolSizes) | result;
    }
    return result;
}

bool DescriptorPools::used(uint32_t& numSets, DescriptorPoolSizes& descriptorPoolSizes) const
{
    bool result = false;
    for (auto& dp : descriptorPools)
    {
        result = dp->used(numSets, descriptorPoolSizes) | result;
    }
    return result;
}

bool DescriptorPools::allocated(uint32_t& numSets, DescriptorPoolSizes& descriptorPoolSizes) const
{
    if (descriptorPools.empty()) return false;

    for (const auto& dp : descriptorPools)
    {
        numSets += dp->maxSets;
        for (auto& dps : dp->descriptorPoolSizes)
        {
            auto itr = std::find_if(descriptorPoolSizes.begin(), descriptorPoolSizes.end(), [&dps](const VkDescriptorPoolSize& value) { return value.type == dps.type; });
            if (itr != descriptorPoolSizes.end())
                itr->descriptorCount += dps.descriptorCount;
            else
                descriptorPoolSizes.push_back(dps);
        }
    }
    return true;
}
