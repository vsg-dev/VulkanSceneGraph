#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/BufferInfo.h>
#include <vsg/state/Descriptor.h>

namespace vsg
{

    /// DescriptorBuffer is Descriptor class that encapsulates bufferInfoList used to set VkWriteDescriptorSet.pBufferInfo settings
    /// DescriptorBuffer is means for passing uniforms to shaders.
    class VSG_DECLSPEC DescriptorBuffer : public Inherit<Descriptor, DescriptorBuffer>
    {
    public:
        DescriptorBuffer();

        explicit DescriptorBuffer(ref_ptr<Data> data, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0, VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        explicit DescriptorBuffer(const DataList& dataList, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0, VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        explicit DescriptorBuffer(const BufferInfoList& in_bufferInfoList, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0, VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

        /// VkWriteDescriptorSet.pBufferInfo settings
        BufferInfoList bufferInfoList;

        int compare(const Object& rhs_object) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context) override;

        void assignTo(Context& context, VkWriteDescriptorSet& wds) const override;

        uint32_t getNumDescriptors() const override;

        void copyDataListToBuffers();

    protected:
        virtual ~DescriptorBuffer();
    };
    VSG_type_name(vsg::DescriptorBuffer)

} // namespace vsg
