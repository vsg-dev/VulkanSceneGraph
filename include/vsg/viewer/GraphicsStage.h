#pragma once

#include <vsg/vk/State.h>
#include <vsg/vk/Framebuffer.h>
#include <vsg/vk/RenderPass.h>

namespace vsg
{

    class VSG_EXPORT GraphicsVisitor : public Visitor
    {
    public:

        GraphicsVisitor(CommandBuffer* commandBuffer);

        ref_ptr<CommandBuffer>  _commandBuffer;
        State                   _state;

        void apply(Node& node);
        void apply(StateGroup& stateGroup);
        void apply(Command& command);

    };

    class VSG_EXPORT GraphicsStage : public Stage
    {
    public:

        GraphicsStage(Node* commandGraph);

        ref_ptr<Node> _commandGraph;

        virtual void populateCommandBuffer(CommandBuffer* commandBuffer, Framebuffer* framebuffer, RenderPass* renderPass,
                                           const VkExtent2D& extent2D, const VkClearColorValue& clearColor) override;

    };

}
