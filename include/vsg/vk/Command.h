#pragma once

#include <vulkan/vulkan.h>

#include <vsg/nodes/Node.h>

namespace vsg
{
    class CommandBuffer;

    class Command : public Node
    {
    public:
        Command() {}

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        virtual void dispatch(CommandBuffer& commandBuffer) const = 0;
    };
}
