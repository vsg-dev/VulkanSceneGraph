/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/vk/DescriptorPools.h>

using namespace vsg;

#define ROLAND_HILL_FIX 0

DescriptorPools::DescriptorPools(ref_ptr<Device> in_device, const ResourceRequirements& in_resourceRequirements) :
    device(in_device)
{
    vsg::info("DescriptorPools::DescriptorPools() ", this);

    minimum_maxSets = in_resourceRequirements.computeNumDescriptorSets();
    minimum_descriptorPoolSizes = in_resourceRequirements.computeDescriptorPoolSizes();

    vsg::info("    minimum_maxSets = ", minimum_maxSets);
    vsg::info("    minimum_descriptorPoolSizes.size() = ", minimum_descriptorPoolSizes.size());
}

DescriptorPools::~DescriptorPools()
{
    vsg::info("DescriptorPools::~DescriptorPools() ", this);
}


void DescriptorPools::getDescriptorPoolSizesToUse(uint32_t& maxSets, DescriptorPoolSizes& descriptorPoolSizes)
{
    vsg::info("descriptorPools::getDescriptorPoolSizesToUse(", maxSets, ", ", descriptorPoolSizes.size(), ") in ");

    if (minimum_maxSets > maxSets)
    {
        maxSets = minimum_maxSets;
    }

#if ROLAND_HILL_FIX
    maxSets = std::max(1u, maxSets);
#endif

    for (auto& [minimum_type, minimum_descriptorCount] : minimum_descriptorPoolSizes)
    {
        auto itr = descriptorPoolSizes.begin();
        for (; itr != descriptorPoolSizes.end(); ++itr)
        {
            if (itr->type == minimum_type)
            {
                if (minimum_descriptorCount > itr->descriptorCount)
                    itr->descriptorCount = minimum_descriptorCount;
                break;
            }
        }
        if (itr == descriptorPoolSizes.end())
        {
            descriptorPoolSizes.push_back(VkDescriptorPoolSize{minimum_type, minimum_descriptorCount});
        }
    }
}

void DescriptorPools::reserve(const ResourceRequirements& requirements)
{
    auto maxSets = requirements.computeNumDescriptorSets();
    auto descriptorPoolSizes = requirements.computeDescriptorPoolSizes();

    uint32_t available_maxSets = 0;
    DescriptorPoolSizes available_descriptorPoolSizes;
    for (auto& descriptorPool : descriptorPools)
    {
        descriptorPool->getAvailability(available_maxSets, available_descriptorPoolSizes);
    }

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

    if (required_maxSets > 0 || !required_descriptorPoolSizes.empty())
    {
        getDescriptorPoolSizesToUse(required_maxSets, required_descriptorPoolSizes);
#if ROLAND_HILL_FIX
        if (required_maxSets > 0 || !required_descriptorPoolSizes.empty())
#else
        if (required_maxSets > 0 && !required_descriptorPoolSizes.empty())
#endif
        {
            descriptorPools.push_back(vsg::DescriptorPool::create(device, required_maxSets, required_descriptorPoolSizes));
        }
        else
        {
            warn("Context::reserve(const ResourceRequirements& requirements) invalid combination of required_maxSets (", required_maxSets, ") & required_descriptorPoolSizes (", required_descriptorPoolSizes.size(), ") unable to allocate DescriptorPool.");
        }
    }
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


