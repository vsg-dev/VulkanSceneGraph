#pragma once

#include <vsg/vk/Buffer.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/nodes/StateGroup.h>

namespace vsg
{

    class VSG_EXPORT BindIndexBuffer : public StateComponent
    {
    public:

        BindIndexBuffer(Buffer* buffer, VkDeviceSize offset, VkIndexType indexType) : _bufferData(buffer, offset, 0), _indexType(indexType) {}

        BindIndexBuffer(const BufferData& bufferData, VkIndexType indexType) : _bufferData(bufferData), _indexType(indexType) {}

        virtual void accept(Visitor& visitor) override { visitor.apply(*this); }

        virtual void pushTo(State& state) override;
        virtual void popFrom(State& state) override;
        virtual void dispatch(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~BindIndexBuffer() {}

        BufferData _bufferData;
        VkIndexType _indexType;

    };

}
