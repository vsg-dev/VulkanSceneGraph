#pragma once

#include <vsg/vk/Buffer.h>
#include <vsg/vk/Command.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/nodes/StateGroup.h>

namespace vsg
{
    class BindVertexBuffers : public StateComponent
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

        virtual void accept(Visitor& visitor) override { visitor.apply(*this); }

        void setFirstBinding(uint32_t firstBinding) { _firstBinding = firstBinding; }
        uint32_t getFirstBinding() const { return _firstBinding; }

        void add(Buffer* buffer, VkDeviceSize offset)
        {
            _buffers.push_back(buffer);
            _vkBuffers.push_back(*buffer);
            _offsets.push_back(offset);
        }

        virtual void pushTo(State& state) override;
        virtual void popFrom(State& state) override;
        virtual void dispatch(CommandBuffer& commandBuffer) const override;

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
