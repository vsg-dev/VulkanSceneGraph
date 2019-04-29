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
#include <vsg/vk/ComputePipeline.h>
#include <vsg/vk/DescriptorSet.h>
#include <vsg/vk/GraphicsPipeline.h>
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

    class MatrixStack
    {
    public:
        MatrixStack(uint32_t in_offset = 0) :
            offset(in_offset)
        {
            // make sure there is an initial matrix
            matrixStack.push(mat4());
            dirty = true;
        }

        using mat4Stack = std::stack<mat4>;
        using dmat4Stack = std::stack<dmat4>;

        mat4Stack matrixStack;
        VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uint32_t offset = 0;
        bool dirty = false;

        inline void set(const mat4& matrix)
        {
            matrixStack = mat4Stack();
            matrixStack.push(matrix);
            dirty = true;
        }

        inline void set(const dmat4& matrix)
        {
            matrixStack = mat4Stack();
            matrixStack.push(matrix);
            dirty = true;
        }

        inline void push(const mat4& matrix)
        {
            matrixStack.push(matrix);
            dirty = true;
        }

        const mat4& top() const { return matrixStack.top(); }

        inline void pop()
        {
            matrixStack.pop();
            dirty = true;
        }

        inline void dispatch(CommandBuffer& commandBuffer)
        {
            if (dirty)
            {
                const PipelineLayout::Implementation* pipelineLayout = commandBuffer.getCurrentPipelineLayout()->implementation();
                vkCmdPushConstants(commandBuffer, *pipelineLayout, stageFlags, offset, sizeof(vsg::mat4), matrixStack.top().data());
                dirty = false;
            }
        }
    };

    class State : public Inherit<Object, State>
    {
    public:
        State() :
            dirty(false) {}

        using ComputePipelineStack = StateStack<BindComputePipeline>;
        using GraphicsPipelineStack = StateStack<BindGraphicsPipeline>;
        using DescriptorStacks = std::vector<StateStack<Command>>;
        using VertexBuffersStack = StateStack<BindVertexBuffers>;
        using IndexBufferStack = StateStack<BindIndexBuffer>;
        using PushConstantsMap = std::map<uint32_t, StateStack<PushConstants>>;

        bool dirty;
        ComputePipelineStack computePipelineStack;
        GraphicsPipelineStack graphicsPipelineStack;

        DescriptorStacks descriptorStacks;

        MatrixStack projectionMatrixStack{0};
        MatrixStack viewMatrixStack{64};
        MatrixStack modelMatrixStack{128};

        PushConstantsMap pushConstantsMap;

        inline void dispatch(CommandBuffer& commandBuffer)
        {
            if (dirty)
            {
                computePipelineStack.dispatch(commandBuffer);
                graphicsPipelineStack.dispatch(commandBuffer);
                for (auto& descriptorStack : descriptorStacks)
                {
                    descriptorStack.dispatch(commandBuffer);
                }

                projectionMatrixStack.dispatch(commandBuffer);
                viewMatrixStack.dispatch(commandBuffer);
                modelMatrixStack.dispatch(commandBuffer);

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
