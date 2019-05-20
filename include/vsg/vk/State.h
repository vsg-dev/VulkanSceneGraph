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

#define MATRIX_STACK_TYPE 0

    template<class T>
    class StateStack
    {
    public:
        StateStack() :
            dirty(false) {}

        using Stack = std::stack<ref_ptr<const T>>;
        Stack stack;
        bool dirty;

        template<class R>
        inline void push(ref_ptr<R> value)
        {
            stack.push(value);
            dirty = true;
        }
        inline void pop()
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
            matrixStack.emplace(matrixStack.top() * matrix);
            dirty = true;
        }

        inline void pushAndPosMult(const AlternativeMatrix& matrix)
        {
            matrixStack.emplace(matrixStack.top() * Matrix(matrix));
            dirty = true;
        }

        inline void pushAndPreMult(const Matrix& matrix)
        {
            matrixStack.emplace(matrixStack.top() * matrix);
            dirty = true;
        }

        inline void pushAndPreMult(const AlternativeMatrix& matrix)
        {
            matrixStack.emplace(matrixStack.top() * Matrix(matrix));
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
        explicit State(CommandBuffer* commandBuffer, uint32_t maxSlot) :
            _commandBuffer(commandBuffer),
            dirty(false),
            stateStacks(maxSlot + 1)
        {
            _frustumUnit = Polytope{{
                Plane(1.0, 0.0, 0.0, 1.0),  // left plane
                Plane(-1.0, 0.0, 0.0, 1.0), // right plane
                Plane(0.0, 1.0, 0.0, 1.0),  // bottom plane
                Plane(0.0, -1.0, 0.0, 1.0)  // top plane
            }};

            _frustumDirty = true;
        }

        using value_type = MatrixStack::value_type;
        using Plane = t_plane<value_type>;
        using Polytope = std::array<Plane, 4>;
        using StateStacks = std::vector<StateStack<StateCommand>>;

        ref_ptr<CommandBuffer> _commandBuffer;

        Polytope _frustumUnit;

        bool _frustumDirty;
        Polytope _frustum;

        bool dirty;

        StateStacks stateStacks;

        MatrixStack projectionMatrixStack{0};
        MatrixStack modelviewMatrixStack{64};

        inline void dispatch()
        {
            if (dirty)
            {
                for (auto& stateStack : stateStacks)
                {
                    stateStack.dispatch(*_commandBuffer);
                }

                projectionMatrixStack.dispatch(*_commandBuffer);
                modelviewMatrixStack.dispatch(*_commandBuffer);

                dirty = false;
            }
        }

        template<typename T>
        bool intersect(t_sphere<T> const& s)
        {
            if (_frustumDirty)
            {
                auto pmv = projectionMatrixStack.top() * modelviewMatrixStack.top();

                _frustum[0] = _frustumUnit[0] * pmv;
                _frustum[1] = _frustumUnit[1] * pmv;
                _frustum[2] = _frustumUnit[2] * pmv;
                _frustum[3] = _frustumUnit[3] * pmv;

                _frustumDirty = false;
            }

            return vsg::intersect(_frustum, s);
        }
    };

} // namespace vsg
