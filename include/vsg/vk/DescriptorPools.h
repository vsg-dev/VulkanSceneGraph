#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/DescriptorSet.h>
#include <vsg/vk/DescriptorPool.h>
#include <vsg/vk/ResourceRequirements.h>

namespace vsg
{

    /// Container for DescriptorPools.
    class VSG_DECLSPEC DescriptorPools : public Inherit<Object, DescriptorPools>
    {
    public:
        explicit DescriptorPools(ref_ptr<Device> in_device, const ResourceRequirements& in_resourceRequirements = {});

        ref_ptr<Device> device;
        uint32_t minimum_maxSets = 0;
        DescriptorPoolSizes minimum_descriptorPoolSizes;
        std::list<ref_ptr<DescriptorPool>> descriptorPools;

        /// check if there are enough Descriptorsets/Descrioptors, if not allocated a new DescriptorPool for these resources
        void reserve(const ResourceRequirements& requirements);

        // allocate vkDescriptorSet
        ref_ptr<DescriptorSet::Implementation> allocateDescriptorSet(DescriptorSetLayout* descriptorSetLayout);

        /// write the internal details to stream.
        void report(std::ostream& out, indentation indent = {}) const;

        /// compute the number of sets and descriptors available.
        bool available(uint32_t& numSets, DescriptorPoolSizes& availableDescriptorPoolSizes) const;

        /// compute the number of sets and descriptors used.
        bool used(uint32_t& numSets, DescriptorPoolSizes& descriptorPoolSizes) const;

        /// compute the number of sets and descriptors allocated.
        bool allocated(uint32_t& numSets, DescriptorPoolSizes& descriptorPoolSizes) const;

    protected:
        virtual ~DescriptorPools();

        /// get the maxSets and descriptorPoolSizes to use
        void getDescriptorPoolSizesToUse(uint32_t& maxSets, DescriptorPoolSizes& descriptorPoolSizes);
    };
    VSG_type_name(vsg::DescriptorPools);

} // namespace vsg