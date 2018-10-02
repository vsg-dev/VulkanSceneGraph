#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Buffer.h>
#include <vsg/vk/Command.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/nodes/StateGroup.h>

namespace vsg
{
    class VSG_EXPORT BindVertexBuffers : public StateComponent
    {
    public:

        BindVertexBuffers() : _firstBinding(0) {}

        BindVertexBuffers(uint32_t firstBinding, const BufferDataList& bufferDataList) : _firstBinding(firstBinding)
        {
            for (auto& bufferData : bufferDataList)
            {
                add(bufferData._buffer, bufferData._offset);
            }
        }

        void accept(Visitor& visitor) override { visitor.apply(*this); }

        void setFirstBinding(uint32_t firstBinding) { _firstBinding = firstBinding; }
        uint32_t getFirstBinding() const { return _firstBinding; }

        void add(Buffer* buffer, VkDeviceSize offset)
        {
            _buffers.push_back(buffer);
            _vkBuffers.push_back(*buffer);
            _offsets.push_back(offset);
        }

        void pushTo(State& state) override;
        void popFrom(State& state) override;
        void dispatch(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~BindVertexBuffers() {}

        using Buffers = std::vector<ref_ptr<Buffer>>;
        using VkBuffers = std::vector<VkBuffer>;
        using Offsets = std::vector<VkDeviceSize>;

        uint32_t _firstBinding;
        Buffers _buffers;
        VkBuffers _vkBuffers;
        Offsets _offsets;

    };
}
