#pragma once

#include <vulkan/vulkan.h>

#include <vsg/nodes/Node.h>

#include <vsg/vk/Command.h>

namespace vsg
{

    class CmdDraw : public Command
    {
    public:
        CmdDraw(uint32_t in_vertexCount, uint32_t in_instanceCount, uint32_t in_firstVertex, uint32_t in_firstInstance):
            vertexCount(in_vertexCount),
            instanceCount(in_instanceCount),
            firstVertex(in_firstVertex),
            firstInstance(in_firstInstance) {}

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        virtual void dispatch(CommandBuffer& commandBuffer) const
        {
            vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        }

        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;
    };

    class CmdDrawIndexed : public Command
    {
    public:
        CmdDrawIndexed(uint32_t in_indexCount, uint32_t in_instanceCount, uint32_t in_firstIndex, int32_t in_vertexOffset, uint32_t in_firstInstance):
            indexCount(in_indexCount),
            instanceCount(in_instanceCount),
            firstIndex(in_firstIndex),
            vertexOffset(in_vertexOffset),
            firstInstance(in_firstInstance) {}

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        virtual void dispatch(CommandBuffer& commandBuffer) const
        {
            vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }

        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t vertexOffset;
        uint32_t firstInstance;
    };
}
