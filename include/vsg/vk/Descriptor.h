#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BufferData.h>
#include <vsg/vk/BufferView.h>
#include <vsg/vk/ImageData.h>

namespace vsg
{
    // forward declare
    class Context;

    using DescriptorBufferInfos = std::vector<VkDescriptorBufferInfo>;

    class Descriptor : public Inherit<Object, Descriptor>
    {
    public:
        Descriptor(uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
            _dstBinding(dstBinding),
            _dstArrayElement(dstArrayElement),
            _descriptorType(descriptorType)
        {
        }

        uint32_t _dstBinding;
        uint32_t _dstArrayElement;
        VkDescriptorType _descriptorType;

        void read(Input& input) override
        {
            Object::read(input);

            input.read("DstBinding", _dstBinding);
            input.read("DstArrayElement", _dstArrayElement);
        }

        void write(Output& output) const override
        {
            Object::write(output);

            output.write("DstBinding", _dstBinding);
            output.write("DstArrayElement", _dstArrayElement);
        }

        // compile the Vulkan object, context parameter used for Device
        virtual void compile(Context& /*context*/) {}

        virtual bool assignTo(VkWriteDescriptorSet& wds, VkDescriptorSet descriptorSet) const
        {
            wds = {};
            wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            wds.dstSet = descriptorSet;
            wds.dstBinding = _dstBinding;
            wds.dstArrayElement = _dstArrayElement;
            wds.descriptorType = _descriptorType;

            return false;
        }

        virtual uint32_t getNumDescriptors() const { return 1; }
    };

    using Descriptors = std::vector<vsg::ref_ptr<vsg::Descriptor>>;

} // namespace vsg
