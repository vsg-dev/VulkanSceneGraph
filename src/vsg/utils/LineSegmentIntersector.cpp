/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Transform.h>
#include <vsg/utils/LineSegmentIntersector.h>

using namespace vsg;

template<typename V>
struct TriangleIntersector
{
    using value_type = V;
    using vec_type = t_vec3<value_type>;

    dvec3 start;
    dvec3 end;
    uint32_t instanceIndex = 0;

    vec_type _d;
    value_type _length;
    value_type _inverse_length;

    vec_type _d_invX;
    vec_type _d_invY;
    vec_type _d_invZ;

    LineSegmentIntersector& intersector;
    ref_ptr<const vec3Array> vertices;

    TriangleIntersector(LineSegmentIntersector& in_intersector, const dvec3& in_start, const dvec3& in_end, ref_ptr<const vec3Array> in_vertices) :
        start(in_start),
        end(in_end),
        intersector(in_intersector),
        vertices(in_vertices)
    {

        _d = end - start;
        _length = length(_d);
        _inverse_length = (_length != 0.0) ? 1.0 / _length : 0.0;
        _d *= _inverse_length;

        _d_invX = _d.x != 0.0 ? _d / _d.x : vec_type(0.0, 0.0, 0.0);
        _d_invY = _d.y != 0.0 ? _d / _d.y : vec_type(0.0, 0.0, 0.0);
        _d_invZ = _d.z != 0.0 ? _d / _d.z : vec_type(0.0, 0.0, 0.0);
    }

    /// intersect with a single triangle
    bool intersect(uint32_t i0, uint32_t i1, uint32_t i2)
    {
        const vec3& v0 = vertices->at(i0);
        const vec3& v1 = vertices->at(i1);
        const vec3& v2 = vertices->at(i2);

        vec_type T = vec_type(start) - vec_type(v0);
        vec_type E2 = vec_type(v2) - vec_type(v0);
        vec_type E1 = vec_type(v1) - vec_type(v0);

        vec_type P = cross(_d, E2);

        value_type det = dot(P, E1);

        value_type r, r0, r1, r2;

        const value_type epsilon = 1e-10;
        if (det > epsilon)
        {
            value_type u = dot(P, T);
            if (u < 0.0 || u > det) return false;

            vec_type Q = cross(T, E1);
            value_type v = dot(Q, _d);
            if (v < 0.0 || v > det) return false;

            if ((u + v) > det) return false;

            value_type inv_det = 1.0 / det;
            value_type t = dot(Q, E2) * inv_det;
            if (t < 0.0 || t > _length) return false;

            u *= inv_det;
            v *= inv_det;

            r0 = 1.0 - u - v;
            r1 = u;
            r2 = v;
            r = t * _inverse_length;
        }
        else if (det < -epsilon)
        {
            value_type u = dot(P, T);
            if (u > 0.0 || u < det) return false;

            vec_type Q = cross(T, E1);
            value_type v = dot(Q, _d);
            if (v > 0.0 || v < det) return false;

            if ((u + v) < det) return false;

            value_type inv_det = 1.0 / det;
            value_type t = dot(Q, E2) * inv_det;
            if (t < 0.0 || t > _length) return false;

            u *= inv_det;
            v *= inv_det;

            r0 = 1.0 - u - v;
            r1 = u;
            r2 = v;
            r = t * _inverse_length;
        }
        else
        {
            return false;
        }

        dvec3 intersection = dvec3(dvec3(v0) * double(r0) + dvec3(v1) * double(r1) + dvec3(v2) * double(r2));
        intersector.add(intersection, double(r), {{i0, r0}, {i1, r1}, {i2, r2}}, instanceIndex);

        return true;
    }
};

LineSegmentIntersector::LineSegmentIntersector(const dvec3& s, const dvec3& e, ref_ptr<ArrayState> initialArrayData) :
    Inherit(initialArrayData)
{
    _lineSegmentStack.push_back(LineSegment{s, e});
}

LineSegmentIntersector::LineSegmentIntersector(const Camera& camera, int32_t x, int32_t y, ref_ptr<ArrayState> initialArrayData) :
    Inherit(initialArrayData)
{
    auto viewport = camera.getViewport();

    vsg::vec2 ndc(0.0f, 0.0f);
    if ((viewport.width > 0) && (viewport.height > 0))
    {
        ndc.set((static_cast<float>(x) - viewport.x) / viewport.width, (static_cast<float>(y) - viewport.y) / viewport.height);
    }

    auto projectionMatrix = camera.projectionMatrix->transform();
    auto viewMatrix = camera.viewMatrix->transform();

    bool reverse_depth = (projectionMatrix(2, 2) > 0.0);

    vsg::dvec3 ndc_near(ndc.x * 2.0 - 1.0, ndc.y * 2.0 - 1.0, reverse_depth ? viewport.maxDepth : viewport.minDepth);
    vsg::dvec3 ndc_far(ndc.x * 2.0 - 1.0, ndc.y * 2.0 - 1.0, reverse_depth ? viewport.minDepth : viewport.maxDepth);

    auto inv_projectionMatrix = vsg::inverse(projectionMatrix);
    vsg::dvec3 eye_near = inv_projectionMatrix * ndc_near;
    vsg::dvec3 eye_far = inv_projectionMatrix * ndc_far;
    _lineSegmentStack.push_back(LineSegment{eye_near, eye_far});

    dmat4 eyeToWorld = inverse(viewMatrix);
    localToWorldStack().push_back(viewMatrix);
    worldToLocalStack().push_back(eyeToWorld);
    _lineSegmentStack.push_back(LineSegment{eyeToWorld * eye_near, eyeToWorld * eye_far});
}

LineSegmentIntersector::Intersection::Intersection(const dvec3& in_localIntersection, const dvec3& in_worldIntersection, double in_ratio, const dmat4& in_localToWorld, const NodePath& in_nodePath, const DataList& in_arrays, const IndexRatios& in_indexRatios, uint32_t in_instanceIndex) :
    localIntersection(in_localIntersection),
    worldIntersection(in_worldIntersection),
    ratio(in_ratio),
    localToWorld(in_localToWorld),
    nodePath(in_nodePath),
    arrays(in_arrays),
    indexRatios(in_indexRatios),
    instanceIndex(in_instanceIndex)
{
}

ref_ptr<LineSegmentIntersector::Intersection> LineSegmentIntersector::add(const dvec3& coord, double ratio, const IndexRatios& indexRatios, uint32_t instanceIndex)
{
    ref_ptr<Intersection> intersection;

    auto localToWorld = computeTransform(_nodePath);
    intersection = Intersection::create(coord, localToWorld * coord, ratio, localToWorld, _nodePath, arrayStateStack.back()->arrays, indexRatios, instanceIndex);
    intersections.emplace_back(intersection);

    return intersection;
}

void LineSegmentIntersector::pushTransform(const Transform& transform)
{
    auto& l2wStack = localToWorldStack();
    auto& w2lStack = worldToLocalStack();

    dmat4 localToWorld = l2wStack.empty() ? transform.transform(dmat4{}) : transform.transform(l2wStack.back());
    dmat4 worldToLocal = inverse(localToWorld);

    l2wStack.push_back(localToWorld);
    w2lStack.push_back(worldToLocal);

    const auto& worldLineSegment = _lineSegmentStack.front();
    _lineSegmentStack.push_back(LineSegment{worldToLocal * worldLineSegment.start, worldToLocal * worldLineSegment.end});
}

void LineSegmentIntersector::popTransform()
{
    _lineSegmentStack.pop_back();
    localToWorldStack().pop_back();
    worldToLocalStack().pop_back();
}

bool LineSegmentIntersector::intersects(const dsphere& bs)
{
    //debug("intersects( center = ", bs.center, ", radius = ", bs.radius, ")");
    if (!bs.valid()) return false;

    const auto& lineSegment = _lineSegmentStack.back();
    const dvec3& start = lineSegment.start;
    const dvec3& end = lineSegment.end;

    dvec3 sm = start - bs.center;
    double c = length2(sm) - bs.radius * bs.radius;
    if (c < 0.0) return true;

    dvec3 se = end - start;
    double a = length2(se);
    double b = dot(sm, se) * 2.0;
    double d = b * b - 4.0 * a * c;

    if (d < 0.0) return false;

    d = sqrt(d);

    double div = 1.0 / (2.0 * a);

    double r1 = (-b - d) * div;
    double r2 = (-b + d) * div;

    if (r1 <= 0.0 && r2 <= 0.0) return false;
    if (r1 >= 1.0 && r2 >= 1.0) return false;

    // passed all the rejection tests so line must intersect bounding sphere, return true.
    return true;
}

bool LineSegmentIntersector::intersectDraw(uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount)
{
    auto& arrayState = *arrayStateStack.back();
    if (arrayState.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST || vertexCount < 3) return false;

    const auto& ls = _lineSegmentStack.back();

    size_t previous_size = intersections.size();
    uint32_t lastIndex = instanceCount > 1 ? (firstInstance + instanceCount) : firstInstance + 1;
    for (uint32_t instanceIndex = firstInstance; instanceIndex < lastIndex; ++instanceIndex)
    {
        TriangleIntersector<double> triIntersector(*this, ls.start, ls.end, arrayState.vertexArray(instanceIndex));
        if (!triIntersector.vertices) return false;

        uint32_t endVertex = int((firstVertex + vertexCount) / 3.0f) * 3;

        for (uint32_t i = firstVertex; i < endVertex; i += 3)
        {
            triIntersector.intersect(i, i + 1, i + 2);
        }
    }

    return intersections.size() != previous_size;
}

bool LineSegmentIntersector::intersectDrawIndexed(uint32_t firstIndex, uint32_t indexCount, uint32_t firstInstance, uint32_t instanceCount)
{
    auto& arrayState = *arrayStateStack.back();
    if (arrayState.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST || indexCount < 3) return false;

    const auto& ls = _lineSegmentStack.back();

    size_t previous_size = intersections.size();
    uint32_t lastIndex = instanceCount > 1 ? (firstInstance + instanceCount) : firstInstance + 1;
    for (uint32_t instanceIndex = firstInstance; instanceIndex < lastIndex; ++instanceIndex)
    {
        TriangleIntersector<double> triIntersector(*this, ls.start, ls.end, arrayState.vertexArray(instanceIndex));
        if (!triIntersector.vertices) continue;

        triIntersector.instanceIndex = instanceIndex;

        uint32_t endIndex = int((firstIndex + indexCount) / 3.0f) * 3;

        if (ushort_indices)
        {
            for (uint32_t i = firstIndex; i < endIndex; i += 3)
            {
                triIntersector.intersect(ushort_indices->at(i), ushort_indices->at(i + 1), ushort_indices->at(i + 2));
            }
        }
        else if (uint_indices)
        {
            for (uint32_t i = firstIndex; i < endIndex; i += 3)
            {
                triIntersector.intersect(uint_indices->at(i), uint_indices->at(i + 1), uint_indices->at(i + 2));
            }
        }
    }

    return intersections.size() != previous_size;
}
