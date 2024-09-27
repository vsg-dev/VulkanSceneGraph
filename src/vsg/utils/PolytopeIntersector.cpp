/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/io/stream.h>
#include <vsg/nodes/Transform.h>
#include <vsg/utils/PrimitiveFunctor.h>
#include <vsg/utils/PolytopeIntersector.h>

#include <iostream>

using namespace vsg;

namespace vsg
{

std::ostream& operator<<(std::ostream& output, const vsg::Polytope& polytope)
{
    output<<"Polytope "<<&polytope<<" {"<<std::endl;
    for(auto& pl : polytope)
    {
        output<<"   "<<pl<<std::endl;
    }
    output<<"}"<<std::endl;
    return output;
}

struct PolytopePrimitiveIntersection
{
    PolytopeIntersector& polyopeIntersector;
    ArrayState& arrayState;
    const Polytope& polytope;
    ref_ptr<const vec3Array> vertices;

    PolytopePrimitiveIntersection(PolytopeIntersector& in_polyopeIntersector, ArrayState& in_arrayState, const Polytope& in_polytope) :
        polyopeIntersector(in_polyopeIntersector),
        arrayState(in_arrayState),
        polytope(in_polytope) {}

    bool instance(uint32_t index)
    {
        info("PolytopePrimitiveIntersection::instance(", index, ")");

        vertices = arrayState.vertexArray(index);
        return vertices.valid();
    }

    void triangle(uint32_t i0, uint32_t i1, uint32_t i2)
    {
        info("PolytopePrimitiveIntersection::triangle(", i0, ", ", i1, ", ", i2, ")");
        const dvec3 v0(vertices->at(i0));
        const dvec3 v1(vertices->at(i1));
        const dvec3 v2(vertices->at(i2));

        if (vsg::inside(polytope, v0) || vsg::inside(polytope, v1) || vsg::inside(polytope, v2))
        {
            info("   inside(", i0, ", ", i1, ", ", i2, ") v0 = ", v0, ", v1 = ", v1, " v2 = ", v2);
        }
    }

    void line(uint32_t i0, uint32_t i1)
    {
        info("PolytopePrimitiveIntersection::line(", i0, ", ", i1, ")");
        const dvec3 v0(vertices->at(i0));
        const dvec3 v1(vertices->at(i1));

        if (vsg::inside(polytope, v0) || vsg::inside(polytope, v1))
        {
            info("   inside(", i0, ", ", i1, ") v0 = ", v0, ", v1 = ", v1);
        }
    }

    void point(uint32_t i0)
    {
        info("PolytopePrimitiveIntersection::point(", i0, ")");
        const dvec3 v0(vertices->at(i0));
        if (vsg::inside(polytope, v0))
        {
            info("   inside(", i0, ") v0 = ", v0);
        }
    }
};

}

PolytopeIntersector::PolytopeIntersector(const Polytope& in_polytope, ref_ptr<ArrayState> initialArrayData) :
    Inherit(initialArrayData)
{
    _polytopeStack.push_back(in_polytope);
}

PolytopeIntersector::PolytopeIntersector(const Camera& camera, double xMin, double yMin, double xMax, double yMax, ref_ptr<ArrayState> initialArrayData) :
    Inherit(initialArrayData)
{
    auto viewport = camera.getViewport();

    info("\nPolytopeIntersector::PolytopeIntersector(camera, ", xMin, ", ", yMin, ", ", xMax, ", ", yMax, ")");

    auto projectionMatrix = camera.projectionMatrix->transform();
    auto viewMatrix = camera.viewMatrix->transform();
    bool reverse_depth = (projectionMatrix(2, 2) > 0.0);

    double ndc_xMin = (viewport.width > 0) ? (2.0 * (xMin - static_cast<double>(viewport.x)) / static_cast<double>(viewport.width) - 1.0) : xMin;
    double ndc_xMax = (viewport.width > 0) ? (2.0 * (xMax - static_cast<double>(viewport.x)) / static_cast<double>(viewport.width) - 1.0) : xMax;

    double ndc_yMin = (viewport.height > 0) ? (2.0 * (yMin - static_cast<double>(viewport.y)) / static_cast<double>(viewport.height) - 1.0) : yMin;
    double ndc_yMax = (viewport.height > 0) ? (2.0 * (yMax - static_cast<double>(viewport.y)) / static_cast<double>(viewport.height) - 1.0) : yMax;

    double ndc_near = reverse_depth ? viewport.maxDepth : viewport.minDepth;
    double ndc_far = reverse_depth ? viewport.minDepth : viewport.maxDepth;

    info("ndc_xMin ", ndc_xMin);
    info("ndc_xMax ", ndc_xMax);
    info("ndc_yMin ", ndc_yMin);
    info("ndc_yMax ", ndc_yMax);
    info("ndc_near ", ndc_near);
    info("ndc_far ", ndc_far);

    vsg::Polytope clipspace;
    clipspace.push_back(dplane(1.0, 0.0, 0.0, -ndc_xMin)); // left
    clipspace.push_back(dplane(-1.0, 0.0, 0.0, ndc_xMax)); // right
    clipspace.push_back(dplane(0.0, 1.0, 0.0, -ndc_yMin)); // bottom
    clipspace.push_back(dplane(0.0, -1.0, 0.0, ndc_yMax)); // top
    clipspace.push_back(dplane(0.0, 0.0, -1.0, ndc_near)); // near
    clipspace.push_back(dplane(0.0, 0.0, 1.0, ndc_far)); // far


    vsg::Polytope eyespace;
    for(auto& pl : clipspace)
    {
        eyespace.push_back(pl * projectionMatrix);
    }

    vsg::Polytope worldspace;
    for(auto& pl : eyespace)
    {
        worldspace.push_back(pl * viewMatrix);
    }

    _polytopeStack.push_back(worldspace);

    std::cout<<"Clip space : "<<clipspace<<std::endl;
    std::cout<<"Eye space : "<<eyespace<<std::endl;
    std::cout<<"World space : "<<worldspace<<std::endl;

}

PolytopeIntersector::Intersection::Intersection(const dvec3& in_localIntersection, const dvec3& in_worldIntersection, double in_ratio, const dmat4& in_localToWorld, const NodePath& in_nodePath, const DataList& in_arrays, const IndexRatios& in_indexRatios, uint32_t in_instanceIndex) :
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

ref_ptr<PolytopeIntersector::Intersection> PolytopeIntersector::add(const dvec3& coord, double ratio, const IndexRatios& indexRatios, uint32_t instanceIndex)
{
    ref_ptr<Intersection> intersection;

    auto localToWorld = computeTransform(_nodePath);
    intersection = Intersection::create(coord, localToWorld * coord, ratio, localToWorld, _nodePath, arrayStateStack.back()->arrays, indexRatios, instanceIndex);
    intersections.emplace_back(intersection);

    return intersection;
}

void PolytopeIntersector::pushTransform(const Transform& transform)
{
    vsg::info("\nPolytopeIntersector::pushTransform(", transform.className(), ")");


    auto& l2wStack = localToWorldStack();
    auto& w2lStack = worldToLocalStack();

    dmat4 localToWorld = l2wStack.empty() ? transform.transform(dmat4{}) : transform.transform(l2wStack.back());
    dmat4 worldToLocal = inverse(localToWorld);

    l2wStack.push_back(localToWorld);
    w2lStack.push_back(worldToLocal);

    const auto& worldspace = _polytopeStack.front();

    Polytope localspace;
    for(auto& pl : worldspace)
    {
        localspace.push_back(pl * localToWorld);
    }

    _polytopeStack.push_back(localspace);
}

void PolytopeIntersector::popTransform()
{
    vsg::info("PolytopeIntersector::popTransform()");

    _polytopeStack.pop_back();
    localToWorldStack().pop_back();
    worldToLocalStack().pop_back();
}

bool PolytopeIntersector::intersects(const dsphere& bs)
{
    //debug("intersects( center = ", bs.center, ", radius = ", bs.radius, ")");
    if (!bs.valid()) return false;

    const auto& polytope = _polytopeStack.back();

    info("PolytopeIntersector::intersects(const dsphere& bs = ", bs.center, ", ", bs.radius, ") : result = ", vsg::intersect(polytope, bs));

    return vsg::intersect(polytope, bs);
}

bool PolytopeIntersector::intersectDraw(uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount)
{
    info("PolytopeIntersector::intersectDraw(", firstVertex, ", ", vertexCount, ", ", firstInstance, ", ", instanceCount,")) todo.");

    size_t previous_size = intersections.size();

    auto& arrayState = *arrayStateStack.back();

    vsg::PrimitiveFunctor<vsg::PolytopePrimitiveIntersection> printPrimitives(*this, arrayState, _polytopeStack.back());
    if (ushort_indices) printPrimitives.draw(arrayState.topology, firstVertex, vertexCount, firstInstance, instanceCount);

    return intersections.size() != previous_size;
}

bool PolytopeIntersector::intersectDrawIndexed(uint32_t firstIndex, uint32_t indexCount, uint32_t firstInstance, uint32_t instanceCount)
{
    info("PolytopeIntersector::intersectDrawIndexed(", firstIndex, ", ", indexCount, ", ", firstInstance, ", ", instanceCount,")) todo.");

    size_t previous_size = intersections.size();

    auto& arrayState = *arrayStateStack.back();

    vsg::PrimitiveFunctor<vsg::PolytopePrimitiveIntersection> printPrimtives(*this, arrayState, _polytopeStack.back());
    if (ushort_indices) printPrimtives.drawIndexed(arrayState.topology, ushort_indices, firstIndex, indexCount, firstInstance, instanceCount);
    else if (uint_indices) printPrimtives.drawIndexed(arrayState.topology, uint_indices, firstIndex, indexCount, firstInstance, instanceCount);

    return intersections.size() != previous_size;
}