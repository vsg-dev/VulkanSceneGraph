#pragma once

#include <vsg/vk/Pipeline.h>
#include <vsg/vk/DescriptorSet.h>
#include <vsg/vk/BindVertexBuffers.h>
#include <vsg/vk/BindIndexBuffer.h>
#include <vsg/vk/PushConstants.h>
#include <vsg/vk/CommandBuffer.h>

#include <stack>

#include <iostream>

namespace vsg
{
    template<class T>
    class StateStack
    {
    public:

        StateStack() : dirty(false) {}

        using Stack = std::stack<ref_ptr<T>>;
        Stack   stack;
        bool    dirty;

        void push(T* value) { stack.push(value); dirty = true; }
        void pop() { stack.pop(); dirty = true; }
        size_t size() const { return stack.size(); }
        T& top() { return stack.top(); }
        const T& top() const { return stack.top(); }

        inline void dispatch(CommandBuffer& commandBuffer)
        {
            if (dirty && !stack.empty())
            {
                std::cout<<"StateStack::dispatch() "<<size()<<" name="<<typeid(T).name()<<std::endl;
                stack.top()->dispatch(commandBuffer);
                dirty = false;
            }
        }

    };

    class State : public Object
    {
    public:
        State() : dirty(false) {}

        bool                            dirty;
        StateStack<BindPipeline>        pipelineStack;
        StateStack<BindDescriptorSets>  descriptorStack;
        StateStack<BindVertexBuffers>   vertexBuffersStack;
        StateStack<BindIndexBuffer>     indexBufferStack;
        StateStack<PushConstants>       pushConstantsStack;

        inline void dispatch(CommandBuffer& commandBuffer)
        {
            std::cout<<"State::dispatch() dirty="<<dirty<<std::endl;
            if (dirty)
            {
                pipelineStack.dispatch(commandBuffer);
                descriptorStack.dispatch(commandBuffer);
                vertexBuffersStack.dispatch(commandBuffer);
                indexBufferStack.dispatch(commandBuffer);
                pushConstantsStack.dispatch(commandBuffer);
                dirty = false;
            }
        }
    };


    class Framebuffer;
    class Renderpass;
    class Stage : public Object
    {
    public:
        Stage() {}

        virtual void populateCommandBuffer(CommandBuffer* commandBuffer, Framebuffer* framebuffer, RenderPass* renderPass, const VkExtent2D& extent, const VkClearColorValue& clearColor) = 0;
    };
}
