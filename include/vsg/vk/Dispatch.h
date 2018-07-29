#pragma once

#include <vulkan/vulkan.h>

#include <vsg/nodes/Node.h>

namespace vsg
{
    class Dispatch : public Node
    {
    public:
        Dispatch() {}

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        virtual void dispatch(VkCommandBuffer commandBuffer) const = 0;
    };
}
