#pragma once

#include <vsg/vk/Buffer.h>
#include <vsg/vk/CmdDraw.h>
#include <vsg/vk/Descriptor.h>

namespace vsg
{


    class BindIndexBuffer : public Command
    {
    public:

        BindIndexBuffer(Buffer* buffer, VkDeviceSize offset, VkIndexType indexType) : _bufferData(buffer, offset, 0), _indexType(indexType) {}

        BindIndexBuffer(const BufferData& bufferData, VkIndexType indexType) : _bufferData(bufferData), _indexType(indexType) {}

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        virtual void dispatch(VkCommandBuffer commandBuffer) const
        {
            vkCmdBindIndexBuffer(commandBuffer, *_bufferData._buffer, _bufferData._offset, _indexType);
        }

    protected:
        virtual ~BindIndexBuffer() {}

        BufferData _bufferData;
        VkIndexType _indexType;

    };
}
