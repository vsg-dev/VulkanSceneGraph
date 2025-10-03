#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2025 Chris Djali

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/VertexDraw.h>
#include <vsg/state/ArrayState.h>

namespace vsg
{
    class LineSegmentIntersector;

    /** IntersectionOptimizedVertexDraw extends VertexDraw with a BVH to accelerate vsg::Intersector operations */
    class VSG_DECLSPEC IntersectionOptimizedVertexDraw : public Inherit<VertexDraw, IntersectionOptimizedVertexDraw>
    {
    public:
        IntersectionOptimizedVertexDraw(const VertexDraw& rhs, const CopyOp& copyop = {});
        IntersectionOptimizedVertexDraw(const IntersectionOptimizedVertexDraw& rhs, const CopyOp& copyop = {});

        void rebuild(ArrayState& arrayState);

        void intersect(LineSegmentIntersector& lineSegmentIntersector) const;

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return IntersectionOptimizedVertexDraw::create(*this, copyop); }
        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        void accept(ConstVisitor& visitor) const override;

    protected:
        virtual ~IntersectionOptimizedVertexDraw();

        struct Triangle
        {
            vec3 vertex0;
            vec3 vertex1;
            vec3 vertex2;
        };
        static constexpr size_t trisPerLeaf = 16;
        struct Leaf
        {
            std::array<Triangle, trisPerLeaf> tris;
        };
        struct NodeRef
        {
            enum NodeType
            {
                LEAF,
                INTERNAL,
                INVALID
            };
            NodeType type;
            uint32_t index;
        };
        struct InternalNode
        {
            std::array<std::pair<box, NodeRef>, 2> children;
        };

        std::vector<InternalNode, allocator_affinity_data<InternalNode>> internalNodes;
        std::vector<Leaf, allocator_affinity_data<Leaf>> leaves;
        box bounds;
        NodeRef boundingVolumeHeirarchy;
    };
    VSG_type_name(vsg::IntersectionOptimizedVertexDraw)

    class VSG_DECLSPEC IntersectionOptimizeVisitor : public Inherit<Visitor, IntersectionOptimizeVisitor>
    {
    public:
        using ArrayStateStack = std::vector<ref_ptr<ArrayState>>;

        IntersectionOptimizeVisitor(ref_ptr<ArrayState> initialArrayState = {});

        void apply(Node& node) override;

        void apply(StateGroup& stateGroup) override;

    protected:
        ArrayStateStack arrayStateStack;
    };
    VSG_type_name(vsg::IntersectionOptimizeVisitor)
} // namespace vsg
