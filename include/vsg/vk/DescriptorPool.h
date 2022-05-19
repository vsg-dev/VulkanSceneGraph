#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/DescriptorSet.h>

namespace vsg
{

    class DescriptorSet_Implementation;

    class VSG_DECLSPEC DescriptorPool : public Inherit<Object, DescriptorPool>
    {
    public:
        DescriptorPool(Device* device, uint32_t maxSets, const DescriptorPoolSizes& descriptorPoolSizes);

        operator const VkDescriptorPool&() const { return _descriptorPool; }

        Device* getDevice() { return _device; }
        const Device* getDevice() const { return _device; }

        ref_ptr<DescriptorSet_Implementation> allocateDescriptorSet(DescriptorSetLayout* descriptorSetLayout);
        void freeDescriptorSet(ref_ptr<DescriptorSet_Implementation> dsi);

        // vkAllocateDescriptorSets(vector<ref_ptr<DescriptorsSetLayout>>);
        // vkAllocateDescriptorSet(ref_ptr<DescriptorsSetLayout>);
        // vkFreeDescriptorSets(VkDescriptorSet)
        // vkUpdateDescriptorSets(...descriptorWrites);

    protected:
        virtual ~DescriptorPool();

        VkDescriptorPool _descriptorPool;
        ref_ptr<Device> _device;
        mutable std::mutex _mutex;

        uint32_t _availableDescriptorSet;
        DescriptorPoolSizes _availableDescriptorPoolSizes;

        std::list<ref_ptr<DescriptorSet_Implementation>> _reclingList;
    };
    VSG_type_name(vsg::DescriptorPool);

    class DescriptorSetLayout;
    class Context;

    struct VSG_DECLSPEC DescriptorSet_Implementation : public Inherit<Object, DescriptorSet_Implementation>
    {
        DescriptorSet_Implementation(DescriptorPool* descriptorPool, DescriptorSetLayout* descriptorSetLayout);
        virtual ~DescriptorSet_Implementation();

        void assign(Context& context, const Descriptors& descriptors);

        VkDescriptorSet _descriptorSet;
        ref_ptr<DescriptorPool> _descriptorPool;
        Descriptors _descriptors;
        DescriptorPoolSizes _descriptorPoolSizes;
    };

    extern VSG_DECLSPEC void recyle(ref_ptr<DescriptorSet_Implementation>& dsi);

} // namespace vsg
