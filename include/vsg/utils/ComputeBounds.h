#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/box.h>
#include <vsg/state/ArrayState.h>

namespace vsg
{

    /// ComputeBounds traverses a scene graph computing an overall bounding box that encloses all the geometry in that scene graph.
    class VSG_DECLSPEC ComputeBounds : public Inherit<ConstVisitor, ComputeBounds>
    {
    public:
        ComputeBounds(ref_ptr<ArrayState> intialArrayState = {});

        /// Result of compute bounds traversal
        dbox bounds;

        /// Use the bounding volumes of Cull/LOD nodes etc to compute the bounds rather than traverse their subgraphs.
        /// Using the bounding volumes is faster but may result in less tight bounds around the geometry in the scene.
        bool useNodeBounds = true;

        using ArrayStateStack = std::vector<ref_ptr<ArrayState>>;
        ArrayStateStack arrayStateStack;

        using MatrixStack = std::vector<dmat4>;
        MatrixStack matrixStack;

        ref_ptr<const ushortArray> ushort_indices;
        ref_ptr<const uintArray> uint_indices;

        void apply(const Object& node) override;
        void apply(const StateGroup& stategroup) override;
        void apply(const Transform& transform) override;
        void apply(const MatrixTransform& transform) override;
        void apply(const CullNode& cullNode) override;
        void apply(const CullGroup& cullGroup) override;
        void apply(const LOD& lod) override;
        void apply(const PagedLOD& plod) override;
        void apply(const Geometry& geometry) override;
        void apply(const VertexDraw& vid) override;
        void apply(const VertexIndexDraw& vid) override;
        void apply(const BindVertexBuffers& bvb) override;
        void apply(const BindIndexBuffer& bib) override;
        void apply(const StateCommand& statecommand) override;
        void apply(const Draw& draw) override;
        void apply(const DrawIndexed& drawIndexed) override;
        void apply(const Text& text) override;
        void apply(const TextGroup& textGroup) override;
        void apply(const TextTechnique& techqniey) override;

        void apply(const BufferInfo& bufferInfo) override;
        void apply(const ushortArray& array) override;
        void apply(const uintArray& array) override;

        virtual void applyDraw(uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount);
        virtual void applyDrawIndexed(uint32_t firstIndex, uint32_t indexCount, uint32_t firstInstance, uint32_t instanceCount);

        void add(const dbox& bb);
        void add(const dsphere& bs);
    };
    VSG_type_name(vsg::ComputeBounds);

} // namespace vsg
