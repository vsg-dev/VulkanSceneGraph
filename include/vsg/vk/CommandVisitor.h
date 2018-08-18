#pragma once

#include <vsg/core/Visitor.h>

#include <vsg/vk/Framebuffer.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/Command.h>

namespace vsg
{

    class CommandVisitor : public Visitor
    {
    public:

        ref_ptr<Framebuffer>    _framebuffer;
        ref_ptr<RenderPass>     _renderPass;
        ref_ptr<CommandBuffer>  _commandBuffer;
        VkExtent2D              _extent;
        VkClearColorValue       _clearColor;

        CommandVisitor(Framebuffer* framebuffer, RenderPass* renderPass, CommandBuffer* commandBuffer, const VkExtent2D& extent, const VkClearColorValue& clearColor);

        void apply(Object& object);

        void apply(Node& object);

        void apply(Command& cmd);

        void populateCommandBuffer(vsg::Node* subgraph);
    };


}
