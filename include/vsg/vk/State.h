#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/plane.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/state/PushConstants.h>
#include <vsg/vk/CommandBuffer.h>

#include <array>
#include <map>
#include <stack>

namespace vsg
{

#define POLYTOPE_SIZE 5

    /// StateStack used internally by vsg::State to manage stack of vsg::StateCommand
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

        template<class R>
        inline void push(R* value)
        {
            stack.push(ref_ptr<const T>(value));
            dirty = true;
        }

        inline void pop()
        {
            stack.pop();
            dirty = !stack.empty();
        }
        size_t size() const { return stack.size(); }
        const T* top() const { return stack.top(); }

        inline void record(CommandBuffer& commandBuffer)
        {
            if (dirty)
            {
                stack.top()->record(commandBuffer);
                dirty = false;
            }
        }
    };

    /// MatrixStack used internally by vsg::State to manage stack of project or modelview matrices
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

        using value_type = double;

        std::stack<dmat4> matrixStack;
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
        inline void push(const Transform& transform)
        {
            matrixStack.emplace(transform.transform(matrixStack.top()));
            dirty = true;
        }

        inline void push(const MatrixTransform& transform)
        {
            matrixStack.emplace(matrixStack.top() * transform.matrix);
            dirty = true;
        }

        const dmat4& top() const { return matrixStack.top(); }

        inline void pop()
        {
            matrixStack.pop();
            dirty = true;
        }

        inline void record(CommandBuffer& commandBuffer)
        {
            if (dirty)
            {
                auto pipeline = commandBuffer.getCurrentPipelineLayout();
                auto stageFlags = commandBuffer.getCurrentPushConstantStageFlags();

                // don't attempt to push matrices if no pipeline is current or no stages are enabled for push constants
                if (pipeline == nullptr || stageFlags == 0)
                {
                    return;
                }

                // make sure matrix is a float matrix.
                mat4 newmatrix(matrixStack.top());
                vkCmdPushConstants(commandBuffer, pipeline, stageFlags, offset, sizeof(newmatrix), newmatrix.data());
                dirty = false;
            }
        }
    };

    /// Frustum used internally by vsg::State to manage view fustum culling during vsg::RecordTraversal
    struct Frustum
    {
        using value_type = MatrixStack::value_type;
        using Plane = t_plane<value_type>;
        using Vector = t_vec4<value_type>;
        Plane face[POLYTOPE_SIZE];
        Vector lodScale;

        Frustum()
        {
            face[0].set(1.0, 0.0, 0.0, 1.0);                                    // left plane
            face[1].set(-1.0, 0.0, 0.0, 1.0);                                   // right plane
            face[2].set(0.0, -1.0, 0.0, 1.0);                                   // bottom plane
            face[3].set(0.0, 1.0, 0.0, 1.0);                                    // top plane
            if constexpr (POLYTOPE_SIZE >= 5) face[4].set(0.0, 0.0, 1.0, 0.0);  // far plane
            if constexpr (POLYTOPE_SIZE >= 6) face[5].set(0.0, 0.0, -1.0, 1.0); // near plane
        }

        template<class M>
        Frustum(const Frustum& pt, const M& matrix)
        {
            face[0] = pt.face[0] * matrix;
            face[1] = pt.face[1] * matrix;
            face[2] = pt.face[2] * matrix;
            face[3] = pt.face[3] * matrix;
            if constexpr (POLYTOPE_SIZE >= 5) face[4] = pt.face[4] * matrix;
            if constexpr (POLYTOPE_SIZE >= 6) face[5] = pt.face[5] * matrix;
        }

        template<class M>
        void set(const Frustum& pt, const M& matrix)
        {
            face[0] = pt.face[0] * matrix;
            face[1] = pt.face[1] * matrix;
            face[2] = pt.face[2] * matrix;
            face[3] = pt.face[3] * matrix;
            if constexpr (POLYTOPE_SIZE >= 5) face[4] = pt.face[4] * matrix;
            if constexpr (POLYTOPE_SIZE >= 6) face[5] = pt.face[5] * matrix;
        }

        template<class M>
        void computeLodScale(const M& proj, const M& mv)
        {
            value_type f = -proj[1][1];
            value_type sc = f * std::sqrt(square(mv[0][0]) + square(mv[1][0]) + square(mv[2][0]) + square(mv[0][1]) + square(mv[1][1]) + square(mv[2][1])) * 0.5;
            value_type inv_scale = value_type(1.0) / sc;
            lodScale.set(mv[0][2] * inv_scale,
                         mv[1][2] * inv_scale,
                         mv[2][2] * inv_scale,
                         mv[3][2] * inv_scale);
        }

        template<typename T>
        bool intersect(const t_sphere<T>& s) const
        {
            auto negative_radius = -s.radius;
            if (distance(face[0], s.center) < negative_radius) return false;
            if (distance(face[1], s.center) < negative_radius) return false;
            if (distance(face[2], s.center) < negative_radius) return false;
            if (distance(face[3], s.center) < negative_radius) return false;
            if constexpr (POLYTOPE_SIZE >= 5)
                if (distance(face[4], s.center) < negative_radius) return false;
            if constexpr (POLYTOPE_SIZE >= 6)
                if (distance(face[5], s.center) < negative_radius) return false;
            return true;
        }
    };

    /// vsg::State used by vsg::RecordTraversal to manage state stacks, projection, modelview matrix and frustum stacks.
    class State : public Inherit<Object, State>
    {
    public:
        explicit State(CommandBuffer* commandBuffer, uint32_t maxSlot) :
            _commandBuffer(commandBuffer),
            dirty(false),
            stateStacks(maxSlot + 1)
        {
        }

        using StateStacks = std::vector<StateStack<StateCommand>>;

        ref_ptr<CommandBuffer> _commandBuffer;

        Frustum _frustumUnit;
        Frustum _frustumProjected;

        using FrustumStack = std::stack<Frustum>;
        FrustumStack _frustumStack;

        bool dirty;

        StateStacks stateStacks;

        MatrixStack projectionMatrixStack{0};
        MatrixStack modelviewMatrixStack{64};

        void setProjectionAndViewMatrix(const dmat4& projMatrix, const dmat4& viewMatrix)
        {
            projectionMatrixStack.set(projMatrix);

            const auto& proj = projectionMatrixStack.top();

            _frustumProjected.set(_frustumUnit, proj);

            modelviewMatrixStack.set(viewMatrix);

            // clear frustum stack
            while (!_frustumStack.empty()) _frustumStack.pop();

            // push frustum in world coords
            pushFrustum();
        }

        inline void record()
        {
            if (dirty)
            {
                for (auto& stateStack : stateStacks)
                {
                    stateStack.record(*_commandBuffer);
                }

                projectionMatrixStack.record(*_commandBuffer);
                modelviewMatrixStack.record(*_commandBuffer);

                dirty = false;
            }
        }

        inline void pushFrustum()
        {
            _frustumStack.push(Frustum(_frustumProjected, modelviewMatrixStack.top()));
            _frustumStack.top().computeLodScale(projectionMatrixStack.top(), modelviewMatrixStack.top());
        }

        inline void applyFrustum()
        {
            _frustumStack.top().set(_frustumProjected, modelviewMatrixStack.top());
            _frustumStack.top().computeLodScale(projectionMatrixStack.top(), modelviewMatrixStack.top());
        }

        inline void popFrustum()
        {
            _frustumStack.pop();
        }

        template<typename T>
        bool intersect(const t_sphere<T>& s) const
        {
            return _frustumStack.top().intersect(s);
        }

        template<typename T>
        T lodDistance(const t_sphere<T>& s) const
        {
            const auto& frustum = _frustumStack.top();
            if (!frustum.intersect(s)) return -1.0;

            const auto& lodScale = frustum.lodScale;
            return std::abs(lodScale[0] * s.x + lodScale[1] * s.y + lodScale[2] * s.z + lodScale[3]);
        }
    };

} // namespace vsg
