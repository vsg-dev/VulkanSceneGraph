#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/BindIndexBuffer.h>
#include <vsg/vk/BindVertexBuffers.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/DescriptorSet.h>
#include <vsg/vk/Pipeline.h>
#include <vsg/vk/PushConstants.h>

#include <map>
#include <stack>

namespace vsg
{
    template<class T>
    class StateStack
    {
    public:
        StateStack() :
            dirty(false) {}

        using Stack = std::stack<ref_ptr<const T>>;
        Stack stack;
        bool dirty;

        void push(const T* value)
        {
            stack.push(ref_ptr<const T>(value));
            dirty = true;
        }
        void pop()
        {
            stack.pop();
            dirty = true;
        }
        size_t size() const { return stack.size(); }
        T& top() { return stack.top(); }
        const T& top() const { return stack.top(); }

        inline void dispatch(CommandBuffer& commandBuffer)
        {
            if (dirty && !stack.empty())
            {
                stack.top()->dispatch(commandBuffer);
                dirty = false;
            }
        }
    };

    class State : public Inherit<Object, State>
    {
    public:
        State() :
            dirty(false) {}

        using PushConstantsMap = std::map<uint32_t, StateStack<PushConstants>>;

        bool dirty;
        StateStack<BindPipeline> pipelineStack;
        StateStack<BindDescriptorSets> descriptorStack;
        StateStack<BindVertexBuffers> vertexBuffersStack;
        StateStack<BindIndexBuffer> indexBufferStack;
        PushConstantsMap pushConstantsMap;

        inline void dispatch(CommandBuffer& commandBuffer)
        {
            if (dirty)
            {
                pipelineStack.dispatch(commandBuffer);
                descriptorStack.dispatch(commandBuffer);
                vertexBuffersStack.dispatch(commandBuffer);
                indexBufferStack.dispatch(commandBuffer);
                for (auto& pushConstantsStack : pushConstantsMap)
                {
                    pushConstantsStack.second.dispatch(commandBuffer);
                }
                dirty = false;
            }
        }
    };

    class Framebuffer;
    class Renderpass;
    class Stage : public Inherit<Object, Stage>
    {
    public:
        Stage() {}

        virtual void populateCommandBuffer(CommandBuffer* commandBuffer, Framebuffer* framebuffer, RenderPass* renderPass, const VkExtent2D& extent, const VkClearColorValue& clearColor) = 0;
    };
} // namespace vsg
