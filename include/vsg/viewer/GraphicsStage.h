#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

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

        using Visitor::apply;

        void apply(Node& node) override;
        void apply(StateGroup& stateGroup) override;
        void apply(Command& command) override;

    };

    class VSG_EXPORT GraphicsStage : public Stage
    {
    public:

        GraphicsStage(Node* commandGraph);

        ref_ptr<Node> _commandGraph;

        void populateCommandBuffer(CommandBuffer* commandBuffer, Framebuffer* framebuffer, RenderPass* renderPass,
                                   const VkExtent2D& extent2D, const VkClearColorValue& clearColor) override;

    };

}
