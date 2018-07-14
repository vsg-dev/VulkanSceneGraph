#pragma once

#include <vulkan/vulkan.h>

#include <vsg/nodes/Node.h>

namespace vsg
{

    class Dispatch : public Node
    {
    public:
        Dispatch() {}

        virtual void dispatch(VkCommandBuffer commandBuffer) const = 0;
    };

    class CmdDraw : public Dispatch
    {
    public:
        CmdDraw(uint32_t in_vertexCount, uint32_t in_instanceCount, uint32_t in_firstVertex, uint32_t in_firstInstance):
            vertexCount(in_vertexCount),
            instanceCount(in_instanceCount),
            firstVertex(in_firstVertex),
            firstInstance(in_firstVertex) {}

        virtual void dispatch(VkCommandBuffer commandBuffer) const
        {
            vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        }

        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;
    };

}
