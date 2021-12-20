#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/Intersector.h>

#include <vsg/viewer/Camera.h>

namespace vsg
{

    struct IndexRatio
    {
        uint32_t index;
        double ratio;
    };

    using IndexRatios = std::vector<IndexRatio>;

    class VSG_DECLSPEC LineSegmentIntersector : public Inherit<Intersector, LineSegmentIntersector>
    {
    public:
        LineSegmentIntersector(const dvec3& s, const dvec3& e, ref_ptr<ArrayState> initialArrayData = {});
        LineSegmentIntersector(const Camera& camera, int32_t x, int32_t y, ref_ptr<ArrayState> initialArrayData = {});

        struct Intersection
        {
            dvec3 localIntersection;
            dvec3 worldIntersection;
            double ratio = 0.0;

            dmat4 localToWord;
            NodePath nodePath;
            DataList arrays;
            IndexRatios indexRatios;

            // return true if Intersection is valid
            operator bool() const { return !nodePath.empty(); }
        };

        using Intersections = std::vector<Intersection>;
        Intersections intersections;

        void add(const dvec3& intersection, double ratio, const IndexRatios& indexRatios);

        void pushTransform(const Transform& transform) override;
        void popTransform() override;

        /// check for intersection intersects with sphere
        bool intersects(const dsphere& bs) override;

        bool intersectDraw(uint32_t firstVertex, uint32_t vertexCount) override;
        bool intersectDrawIndexed(uint32_t firstIndex, uint32_t indexCount) override;

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
