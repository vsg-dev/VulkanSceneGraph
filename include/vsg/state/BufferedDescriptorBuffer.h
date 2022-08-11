#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/DescriptorBuffer.h>

namespace vsg
{
    /// dynamic version of DescriptorBuffer with extra buffer space to be used with BindDynamicDescriptorSet. 
    /// If you have state that changes from frame to frame then it's important to buffer it to make sure that queued frames don't overwrite each other.
    class VSG_DECLSPEC BufferedDescriptorBuffer : public Inherit<DescriptorBuffer, BufferedDescriptorBuffer>
    {
    public:
        BufferedDescriptorBuffer();

        explicit BufferedDescriptorBuffer(ref_ptr<Data> data, uint32_t in_dstBinding = 0, uint32_t in_dstArrayElement = 0, VkDescriptorType in_descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) : Inherit(data, in_dstBinding, in_dstArrayElement, in_descriptorType) {}
        explicit BufferedDescriptorBuffer(const DataList& dataList, uint32_t in_dstBinding = 0, uint32_t in_dstArrayElement = 0, VkDescriptorType in_descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) : Inherit(dataList, in_dstBinding, in_dstArrayElement, in_descriptorType) {}
        explicit BufferedDescriptorBuffer(const BufferInfoList& in_bufferInfoList, uint32_t in_dstBinding, uint32_t in_dstArrayElement, VkDescriptorType in_descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) : Inherit(in_bufferInfoList, in_dstBinding, in_dstArrayElement, in_descriptorType) {}

        uint32_t numBuffers = 3;

        void compile(Context& context) override;

        /// Automatically called by copyDataListToBuffers
        void advanceFrame();

        void copyDataListToBuffers();

    protected:
        virtual ~BufferedDescriptorBuffer();
        int _frameIndex = -1;
    };
    VSG_type_name(vsg::BufferedDescriptorBuffer)

} // namespace vsg
