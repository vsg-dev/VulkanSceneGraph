#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Node.h>

#include <iostream>
#include <algorithm>
#include <mutex>


namespace vsg
{

class Draw : public vsg::Node
{
public:
    // primitive type?

    Draw(uint32_t in_vertexCount, uint32_t in_instanceCount, uint32_t in_firstVertex, uint32_t in_firstInstance):
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

int main(int argc, char** argv)
{

    // three vertices, 1 instance, starting at 0,0
    vsg::ref_ptr<vsg::Draw> draw = new vsg::Draw(3, 1, 0, 0);



    return 0;
}