#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <memory>
#include <vsg/core/Object.h>

#include <vsg/nodes/Group.h>

#include <vsg/vk/CommandPool.h>
#include <vsg/vk/GraphicsPipeline.h>
#include <vsg/vk/DescriptorPool.h>

namespace vsg
{
    struct Context
    {
        ref_ptr<Device> device;
        ref_ptr<CommandPool> commandPool;
        ref_ptr<RenderPass> renderPass;
        ref_ptr<ViewportState> viewport;
        VkQueue graphicsQueue = 0;

        ref_ptr<DescriptorPool> descriptorPool;

        ref_ptr<mat4Value> projMatrix;
        ref_ptr<mat4Value> viewMatrix;
    };

    class GraphicsNode : public Inherit<Group, GraphicsNode>
    {
    public:
        GraphicsNode(Allocator* allocator = nullptr):
            Inherit(allocator) {}

        void read(Input& input) override;
        void write(Output& output) const override;

        virtual void compile(Context& context) = 0;
    };
    VSG_type_name(vsg::GraphicsNode)


    class VSG_DECLSPEC CompileTraversal : public Visitor
    {
    public:
        explicit CompileTraversal();
        ~CompileTraversal();

        void apply(Object& object);
        void apply(Command& command);
        void apply(Group& group);
        void apply(StateGroup& stateGroup);
        void apply(GraphicsNode& graphics);

        Context context;
    };
} // namespace vsg
