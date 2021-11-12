/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/traversals/LineSegmentIntersector.h>
#include <vsg/nodes/Transform.h>

using namespace vsg;

template<typename V>
struct TriangleIntersector
{
    using value_type = V;
    using vec_type = t_vec3<value_type>;

    dvec3 start;
    dvec3 end;

    vec_type _d;
    value_type _length;
    value_type _inverse_length;

    vec_type _d_invX;
    vec_type _d_invY;
    vec_type _d_invZ;

    LineSegmentIntersector& intersector;
    ref_ptr<const vec3Array> vertices;

    TriangleIntersector(LineSegmentIntersector& in_intersector, const dvec3& in_start, const dvec3& in_end, ref_ptr<const vec3Array> in_vertices) :
        intersector(in_intersector),
        vertices(in_vertices)
    {
        start = in_start;
        end = in_end;

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

        // TODO : handle hit

        dvec3 intersection = dvec3(dvec3(v0) * double(r0) + dvec3(v1) * double(r1) + dvec3(v2) * double(r2));
        intersector.add(intersection, double(r), {{i0, r0}, {i1, r1}, {i2, r2}});

        return true;
    }
};

LineSegmentIntersector::LineSegmentIntersector(const dvec3& s, const dvec3& e)
{
    _lineSegmentStack.push_back(LineSegment{s, e});
}

LineSegmentIntersector::LineSegmentIntersector(const Camera& camera, int32_t x, int32_t y)
{
    auto viewport = camera.getViewport();

    vsg::vec2 ndc(0.0f, 0.0f);
    if ((viewport.width > 0) && (viewport.height > 0))
    {
        ndc.set((static_cast<float>(x) - viewport.x) / viewport.width, (static_cast<float>(y) - viewport.y) / viewport.height);
    }

    vsg::dvec3 ndc_near(ndc.x * 2.0 - 1.0, ndc.y * 2.0 - 1.0, viewport.minDepth * 2.0 - 1.0);
    vsg::dvec3 ndc_far(ndc.x * 2.0 - 1.0, ndc.y * 2.0 - 1.0, viewport.maxDepth * 2.0 - 1.0);

    auto projectionMatrix = camera.projectionMatrix->transform();
    auto viewMatrix = camera.viewMatrix->transform();
    auto inv_projectionViewMatrix = vsg::inverse(projectionMatrix * viewMatrix);

    vsg::dvec3 world_near = inv_projectionViewMatrix * ndc_near;
    vsg::dvec3 world_far = inv_projectionViewMatrix * ndc_far;

    _lineSegmentStack.push_back(LineSegment{world_near, world_far});
}

void LineSegmentIntersector::add(const dvec3& intersection, double ratio, const IndexRatios& indexRatios)
{
    if (_matrixStack.empty())
    {
        intersections.emplace_back(Intersection{intersection, intersection, ratio, {}, _nodePath, arrayStateStack.back().arrays, indexRatios});
    }
    else
    {
        auto& localToWorld = _matrixStack.back();
        intersections.emplace_back(Intersection{intersection, localToWorld * intersection, ratio, localToWorld, _nodePath, arrayStateStack.back().arrays, indexRatios});
    }
}

void LineSegmentIntersector::pushTransform(const Transform& transform)
{
    dmat4 localToWorld = _matrixStack.empty() ? transform.transform(dmat4{}) : transform.transform(_matrixStack.back());
    dmat4 worldToLocal = inverse(localToWorld);

    _matrixStack.push_back(localToWorld);

    auto& worldLineSegment = _lineSegmentStack.front();
    _lineSegmentStack.push_back(LineSegment{worldToLocal * worldLineSegment.start, worldToLocal * worldLineSegment.end});
}

void LineSegmentIntersector::popTransform()
{
    _lineSegmentStack.pop_back();
    _matrixStack.pop_back();
}

bool LineSegmentIntersector::intersects(const dsphere& bs)
{
    //std::cout<<"intersects( center = "<<bs.center<<", radius = "<<bs.radius<<")"<<std::endl;
    if (!bs.valid()) return false;

    auto& lineSegment = _lineSegmentStack.back();
    dvec3& start = lineSegment.start;
    dvec3& end = lineSegment.end;

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

bool LineSegmentIntersector::intersectDraw(uint32_t firstVertex, uint32_t vertexCount)
{
    auto& arrayState = arrayStateStack.back();
    if (!arrayState.vertices || arrayState.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST || vertexCount == 0) return false;

    auto& ls = _lineSegmentStack.back();

    TriangleIntersector<double> triIntsector(*this, ls.start, ls.end, arrayState.vertices);
    if (!triIntsector.vertices) return false;

    size_t previous_size = intersections.size();
    uint32_t endVertex = firstVertex + vertexCount;

    for (uint32_t i = firstVertex; i < endVertex; i += 3)
    {
        triIntsector.intersect(i, i + 1, i + 2);
    }

    return intersections.size() != previous_size;
}

bool LineSegmentIntersector::intersectDrawIndexed(uint32_t firstIndex, uint32_t indexCount)
{
    auto& arrayState = arrayStateStack.back();
    if (!arrayState.vertices || arrayState.topology != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST || indexCount == 0) return false;

    auto& ls = _lineSegmentStack.back();

    TriangleIntersector<double> triIntsector(*this, ls.start, ls.end, arrayState.vertices);
    if (!triIntsector.vertices) return false;

    size_t previous_size = intersections.size();
    uint32_t endIndex = firstIndex + indexCount;

    if (ushort_indices)
    {
        for (uint32_t i = firstIndex; i < endIndex; i += 3)
        {
            triIntsector.intersect(ushort_indices->at(i), ushort_indices->at(i + 1), ushort_indices->at(i + 2));
        }
    }
    else if (uint_indices)
    {
        for (uint32_t i = firstIndex; i < endIndex; i += 3)
        {
            triIntsector.intersect(uint_indices->at(i), uint_indices->at(i + 1), uint_indices->at(i + 2));
        }
    }

    return intersections.size() != previous_size;
}
