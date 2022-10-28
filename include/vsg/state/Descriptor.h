#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>

namespace vsg
{
    // forward declare
    class Context;

    using DescriptorBufferInfos = std::vector<VkDescriptorBufferInfo>;

    /// Descriptor base class for descriptor DescriptorBuffer/DescriptorImage/DescriptorTexelBufferView classes.
    /// Descriptors are assigned BindDescriptorState state commands.
    /// Provides VkWriteDescriptorSet settings that are required for all types of descriptors.
    class VSG_DECLSPEC Descriptor : public Inherit<Object, Descriptor>
    {
    public:
        Descriptor(uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType);

        /// Common VkWriteDescriptorSet settings
        uint32_t dstBinding;
        uint32_t dstArrayElement;
        VkDescriptorType descriptorType;

        int compare(const Object& rhs_object) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        // compile the Vulkan object, context parameter used for Device
        virtual void compile(Context& /*context*/) {}

        virtual void assignTo(Context& context, VkWriteDescriptorSet& wds) const;

        virtual uint32_t getNumDescriptors() const { return 1; }
    };
    VSG_type_name(vsg::Descriptor);

    using Descriptors = std::vector<vsg::ref_ptr<vsg::Descriptor>>;

} // namespace vsg
