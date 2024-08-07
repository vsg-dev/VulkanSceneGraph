#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/stream.h>
#include <vsg/state/DescriptorSet.h>

namespace vsg
{

    /// DescriptorPool encapsulates management of VkDescriptorPool
    class VSG_DECLSPEC DescriptorPool : public Inherit<Object, DescriptorPool>
    {
    public:
        DescriptorPool(Device* device, uint32_t in_maxSets, const DescriptorPoolSizes& in_descriptorPoolSizes);

        operator VkDescriptorPool() const { return _descriptorPool; }
        VkDescriptorPool vk() const { return _descriptorPool; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

        /// allocate or reuse available DescriptorSet::Implementation - called automatically when compiling DescriptorSet
        ref_ptr<DescriptorSet::Implementation> allocateDescriptorSet(DescriptorSetLayout* descriptorSetLayout);

        /// free DescriptorSet::Implementation for reuse - called automatically by destruction of DescriptorSet or release of its Vulkan resources.
        void freeDescriptorSet(ref_ptr<DescriptorSet::Implementation> dsi);

        /// get the stats of the available DescriptorSets/Descriptors
        bool available(uint32_t& numSets, DescriptorPoolSizes& descriptorPoolSizes) const;

        /// compute the number of sets and descriptors used.
        bool used(uint32_t& numSets, DescriptorPoolSizes& usedDescriptorPoolSizes) const;

        /// write the internal details to stream.
        void report(std::ostream& out, indentation indent = {}) const;

        const uint32_t maxSets = 0;
        const DescriptorPoolSizes descriptorPoolSizes;

        /// mutex used to ensure thread safe access of DescriptorPool resources.
        /// Locked automatically by allocateDescriptorSet(..), freeDescriptorSet(), available() and DescriptorSet:::Implementation
        /// to ensure thread safe operation. Normal VulkanSceneGraph usage will not require users to lock this mutex so treat as an internal implementation detail.
        mutable std::mutex mutex;

    protected:
        virtual ~DescriptorPool();

        VkDescriptorPool _descriptorPool;
        ref_ptr<Device> _device;

        uint32_t _availableDescriptorSet;
        DescriptorPoolSizes _availableDescriptorPoolSizes;

        std::list<ref_ptr<DescriptorSet::Implementation>> _recyclingList;
    };
    VSG_type_name(vsg::DescriptorPool);

} // namespace vsg
