#pragma once

#include <vulkan/vulkan.h>

#include <vsg/nodes/Node.h>
#include <vsg/vk/CommandBuffer.h>

namespace vsg
{
    class Command : public Node
    {
    public:
        Command() {}

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        virtual void dispatch(CommandBuffer& commandBuffer) const = 0;
    };
}
