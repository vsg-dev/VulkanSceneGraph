#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/Camera.h>
#include <vsg/maths/plane.h>
#include <vsg/utils/Intersector.h>
#include <vsg/utils/LineSegmentIntersector.h>

namespace vsg
{

    using Polytope = std::vector<dplane>;

    /// PolytopeIntersector is an Intersector subclass that provides support for computing intersections between a line segment and geometry in the scene graph.
    class VSG_DECLSPEC PolytopeIntersector : public Inherit<Intersector, PolytopeIntersector>
    {
    public:
        /// create intersector for specified polytope.
        explicit PolytopeIntersector(const Polytope& in_polytope, ref_ptr<ArrayState> initialArrayData = {});

        /// create intersector for a polytope with window space dimensions, projected into world coords using the Camera's projection and view matrices.
        PolytopeIntersector(const Camera& camera, double xMin, double yMin, double xMax, double yMax, ref_ptr<ArrayState> initialArrayData = {});

        void reset(ref_ptr<ArrayState> initialArrayData = {}) override;
        virtual void reset(const Polytope& in_polytope, ref_ptr<ArrayState> initialArrayData = {});
        virtual void reset(const Camera& camera, double xMin, double yMin, double xMax, double yMax, ref_ptr<ArrayState> initialArrayData = {});

        class VSG_DECLSPEC Intersection : public Inherit<Object, Intersection>
        {
        public:
            Intersection() {}
            Intersection(const dvec3& in_localIntersection, const dvec3& in_worldIntersection, const dmat4& in_localToWorld, const NodePath& in_nodePath, const DataList& in_arrays, const std::vector<uint32_t>& in_indexRatios, uint32_t in_instanceIndex);

            dvec3 localIntersection;
            dvec3 worldIntersection;

            dmat4 localToWorld;
            NodePath nodePath;
            DataList arrays;
            std::vector<uint32_t> indices;
            uint32_t instanceIndex = 0;

            // return true if Intersection is valid
            operator bool() const { return !nodePath.empty(); }
        };

        using Intersections = std::vector<ref_ptr<Intersection>>;
        Intersections intersections;

        ref_ptr<Intersection> add(const dvec3& coord, const std::vector<uint32_t>& indices, uint32_t instanceIndex);

        void pushTransform(const Transform& transform) override;
        void popTransform() override;

        /// check for intersection with sphere
        bool intersects(const dsphere& bs) override;

        bool intersectDraw(uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount) override;
        bool intersectDrawIndexed(uint32_t firstIndex, uint32_t indexCount, uint32_t firstInstance, uint32_t instanceCount) override;

    protected:
        std::vector<Polytope> _polytopeStack;
    };
    VSG_type_name(vsg::PolytopeIntersector);

} // namespace vsg
