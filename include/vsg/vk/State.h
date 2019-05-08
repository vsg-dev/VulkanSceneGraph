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

    #define USE_DOUBLE_MATRIX_STACK 0
    #define USE_COMPUTE_PIPELIE_STACK 1
    #define USE_PUSH_CONSTNANT_STACK 1

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
            dirty = !stack.empty();
        }
        size_t size() const { return stack.size(); }
        T& top() { return stack.top(); }
        const T& top() const { return stack.top(); }

        inline void dispatch(CommandBuffer& commandBuffer)
        {
            if (dirty)
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
            matrixStack.emplace(mat4());
            dirty = true;
        }

#if USE_DOUBLE_MATRIX_STACK
        using value_type = double;
        using alternative_type = float;
#else
        using value_type = float;
        using alternative_type = double;
#endif

        using Matrix = t_mat4<value_type>;
        using AlternativeMatrix = t_mat4<alternative_type>;

        std::stack<Matrix> matrixStack;
        VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uint32_t offset = 0;
        bool dirty = false;

        inline void set(const mat4& matrix)
        {
            matrixStack = {};
            matrixStack.emplace(matrix);
            dirty = true;
        }

        inline void set(const dmat4& matrix)
        {
            matrixStack = {};
            matrixStack.emplace(matrix);
            dirty = true;
        }

        inline void push(const mat4& matrix)
        {
            matrixStack.emplace(matrix);
            dirty = true;
        }
        inline void push(const dmat4& matrix)
        {
            matrixStack.emplace(matrix);
            dirty = true;
        }

        inline void pushAndPosMult(const Matrix& matrix)
        {
            matrixStack.emplace( matrixStack.top() * matrix );
            dirty = true;
        }

        inline void pushAndPosMult(const AlternativeMatrix& matrix)
        {
            matrixStack.emplace( matrixStack.top() * Matrix(matrix) );
            dirty = true;
        }

        inline void pushAndPreMult(const Matrix& matrix)
        {
            matrixStack.emplace( matrixStack.top() * matrix );
            dirty = true;
        }

        inline void pushAndPreMult(const AlternativeMatrix& matrix)
        {
            matrixStack.emplace( matrixStack.top() * Matrix(matrix) );
            dirty = true;
        }

        const Matrix& top() const { return matrixStack.top(); }

        inline void pop()
        {
            matrixStack.pop();
            dirty = true;
        }

        inline void dispatch(CommandBuffer& commandBuffer)
        {
            if (dirty)
            {
#if USE_DOUBLE_MATRIX_STACK
                // make sure matrix is a float matrix.
                mat4 newmatrix(matrixStack.top());
                vkCmdPushConstants(commandBuffer, commandBuffer.getCurrentPipelineLayout(), stageFlags, offset, sizeof(newmatrix), newmatrix.data());
#else
                vkCmdPushConstants(commandBuffer, commandBuffer.getCurrentPipelineLayout(), stageFlags, offset, sizeof(Matrix), matrixStack.top().data());
#endif
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
#if USE_COMPUTE_PIPELIE_STACK
        ComputePipelineStack computePipelineStack;
#endif

        GraphicsPipelineStack graphicsPipelineStack;

        DescriptorStacks descriptorStacks;

        MatrixStack projectionMatrixStack{0};
        MatrixStack modelviewMatrixStack{64};

#if USE_PUSH_CONSTNANT_STACK
        PushConstantsMap pushConstantsMap;
#endif

        inline void dispatch(CommandBuffer& commandBuffer)
        {
            if (dirty)
            {
#if USE_COMPUTE_PIPELIE_STACK
                computePipelineStack.dispatch(commandBuffer);
#endif
                graphicsPipelineStack.dispatch(commandBuffer);
                for (auto& descriptorStack : descriptorStacks)
                {
                    descriptorStack.dispatch(commandBuffer);
                }

                projectionMatrixStack.dispatch(commandBuffer);
                modelviewMatrixStack.dispatch(commandBuffer);

#if USE_PUSH_CONSTNANT_STACK
                for (auto& pushConstantsStack : pushConstantsMap)
                {
                    pushConstantsStack.second.dispatch(commandBuffer);
                }
#endif
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
