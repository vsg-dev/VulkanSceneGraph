#pragma once

#include <vulkan/vulkan.h>

#include <vsg/nodes/Node.h>

namespace vsg
{

class CmdDraw : public vsg::Node
{
public:
    // primitive type?

    CmdDraw(uint32_t in_vertexCount, uint32_t in_instanceCount, uint32_t in_firstVertex, uint32_t in_firstInstance):
        vertexCount(in_vertexCount),
        instanceCount(in_instanceCount),
        firstVertex(in_firstVertex),
        firstInstance(in_firstVertex) {}

    inline void draw(VkCommandBuffer commandBuffer) const
    {
        // enable draw's to be assigned to ckCommandBuffers decided by the draw traversal.

        vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};

}
