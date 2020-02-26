#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/StateGroup.h>
#include <vsg/vk/Buffer.h>
#include <vsg/vk/Command.h>
#include <vsg/vk/Descriptor.h>

namespace vsg
{
    class VSG_DECLSPEC BindVertexBuffers : public Inherit<Command, BindVertexBuffers>
    {
    public:
        BindVertexBuffers() :
            _firstBinding(0) {}

        BindVertexBuffers(uint32_t firstBinding, const DataList& arrays) :
            _firstBinding(firstBinding),
            _arrays(arrays) {}


        BindVertexBuffers(uint32_t firstBinding, const BufferDataList& bufferDataList) :
            _firstBinding(firstBinding)
        {
            for (auto& bufferData : bufferDataList)
            {
                add(bufferData._buffer, bufferData._offset);
            }
        }

        void setFirstBinding(uint32_t firstBinding) { _firstBinding = firstBinding; }
        uint32_t getFirstBinding() const { return _firstBinding; }

        void add(ref_ptr<Buffer> buffer, VkDeviceSize offset);

        void setArrays(const DataList& arrays) { _arrays = arrays; }
        DataList& getArrays() { return _arrays; }
        const DataList& getArrays() const { return _arrays; }

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context) override;

        void dispatch(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~BindVertexBuffers();

        uint32_t _firstBinding;
        DataList _arrays;

        struct VulkanData
        {
            std::vector<ref_ptr<Buffer>> buffers;
            std::vector<VkBuffer> vkBuffers;
            std::vector<VkDeviceSize> offsets;
        };

        VulkanData& getVulkanData(uint32_t deviceID)
        {
            if (deviceID >= _vulkanData.size()) _vulkanData.resize(deviceID+1);
            return _vulkanData[deviceID];
        }

        std::vector<VulkanData> _vulkanData;

    };
    VSG_type_name(vsg::BindVertexBuffers);

} // namespace vsg
