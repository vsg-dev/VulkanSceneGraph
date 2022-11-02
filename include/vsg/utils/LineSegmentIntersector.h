#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/Camera.h>
#include <vsg/utils/Intersector.h>

namespace vsg
{

    /// IndexRatio is a pair of index and ratio used specify the baricentric coords of primitives that have been intersected.
    struct IndexRatio
    {
        uint32_t index;
        double ratio;
    };

    using IndexRatios = std::vector<IndexRatio>;

    /// LineSegmentIntersector is a Intersector subclass that provides support for computing intersections between a line segment and geometry in the scene graph.
    class VSG_DECLSPEC LineSegmentIntersector : public Inherit<Intersector, LineSegmentIntersector>
    {
    public:
        LineSegmentIntersector(const dvec3& s, const dvec3& e, ref_ptr<ArrayState> initialArrayData = {});
        LineSegmentIntersector(const Camera& camera, int32_t x, int32_t y, ref_ptr<ArrayState> initialArrayData = {});

        class VSG_DECLSPEC Intersection : public Inherit<Object, Intersection>
        {
        public:
            Intersection() {}
            Intersection(const dvec3& in_localIntersection, const dvec3& in_worldIntersection, double in_ratio, const dmat4& in_localToWorld, const NodePath& in_nodePath, const DataList& in_arrays, const IndexRatios& in_indexRatios, uint32_t in_instanceIndex);

            dvec3 localIntersection;
            dvec3 worldIntersection;
            double ratio = 0.0;

            dmat4 localToWorld;
            NodePath nodePath;
            DataList arrays;
            IndexRatios indexRatios;
            uint32_t instanceIndex = 0;

            // return true if Intersection is valid
            operator bool() const { return !nodePath.empty(); }
        };

        using Intersections = std::vector<ref_ptr<Intersection>>;
        Intersections intersections;

        ref_ptr<Intersection> add(const dvec3& coord, double ratio, const IndexRatios& indexRatios, uint32_t instanceIndex);

        void pushTransform(const Transform& transform) override;
        void popTransform() override;

        /// check for intersection intersects with sphere
        bool intersects(const dsphere& bs) override;

        bool intersectDraw(uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount) override;
        bool intersectDrawIndexed(uint32_t firstIndex, uint32_t indexCount, uint32_t firstInstance, uint32_t instanceCount) override;

    protected:
        struct LineSegment
        {
            dvec3 start;
            dvec3 end;
        };

        std::vector<LineSegment> _lineSegmentStack;
    };
    VSG_type_name(vsg::LineSegmentIntersector);

} // namespace vsg
