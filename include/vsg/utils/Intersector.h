#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Node.h>
#include <vsg/state/ArrayState.h>

namespace vsg
{

    /// Intersector is a base class for intersecting the scene graph.
    /// The LineSegmentIntersector subclass adds support for line segment intersections.
    class VSG_DECLSPEC Intersector : public Inherit<ConstVisitor, Intersector>
    {
    public:
        using NodePath = std::vector<const Node*>;
        using ArrayStateStack = std::vector<ref_ptr<ArrayState>>;

        Intersector(ref_ptr<ArrayState> initialArrayState = {});

        //
        // handle traverse of the scene graph
        //
        void apply(const Node& node) override;
        void apply(const StateGroup& stategroup) override;
        void apply(const Transform& transform) override;
        void apply(const LOD& lod) override;
        void apply(const PagedLOD& plod) override;
        void apply(const CullNode& cn) override;
        void apply(const CullGroup& cn) override;
        void apply(const DepthSorted& cn) override;

        void apply(const VertexDraw& vid) override;
        void apply(const VertexIndexDraw& vid) override;
        void apply(const Geometry& geometry) override;

        void apply(const BindVertexBuffers& bvb) override;
        void apply(const BindIndexBuffer& bib) override;
        void apply(const Draw& draw) override;
        void apply(const DrawIndexed& drawIndexed) override;

        void apply(const BufferInfo& bufferInfo) override;
        void apply(const ubyteArray& array) override;
        void apply(const ushortArray& array) override;
        void apply(const uintArray& array) override;

        void apply(const TextTechnique& technique) override;

        //
        // provide virtual functions for concrete Intersector implementations to provide handling of intersection with mesh geometries
        //

        /// clone and transform this Intersector to provide a new Intersector in local coordinates
        virtual void pushTransform(const Transform& transform) = 0;

        virtual void popTransform() = 0;

        /// check for intersection with sphere
        virtual bool intersects(const dsphere& sphere) = 0;

        /// intersect with a vkCmdDraw primitive
        virtual bool intersectDraw(uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount) = 0;

        /// intersect with a vkCmdDrawIndexed primitive
        virtual bool intersectDrawIndexed(uint32_t firstIndex, uint32_t indexCount, uint32_t firstInstance, uint32_t instanceCount) = 0;

        /// get the current local to world matrix stack
        std::vector<dmat4>& localToWorldStack() { return arrayStateStack.back()->localToWorldStack; }

        /// get the current world to local matrix stack
        std::vector<dmat4>& worldToLocalStack() { return arrayStateStack.back()->worldToLocalStack; }

    protected:
        ArrayStateStack arrayStateStack;

        ref_ptr<const ubyteArray> ubyte_indices;
        ref_ptr<const ushortArray> ushort_indices;
        ref_ptr<const uintArray> uint_indices;

        NodePath _nodePath;
    };
    VSG_type_name(vsg::Intersector);

} // namespace vsg
