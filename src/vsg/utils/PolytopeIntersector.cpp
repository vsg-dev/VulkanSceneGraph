/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/stream.h>
#include <vsg/nodes/Transform.h>
#include <vsg/utils/PolytopeIntersector.h>
#include <vsg/utils/PrimitiveFunctor.h>

#include <iostream>

using namespace vsg;

namespace vsg
{

    std::ostream& operator<<(std::ostream& output, const vsg::Polytope& polytope)
    {
        output << "Polytope " << &polytope << " {" << std::endl;
        for (const auto& pl : polytope)
        {
            output << "   " << pl << std::endl;
        }
        output << "}" << std::endl;
        return output;
    }

    struct PolytopePrimitiveIntersection
    {
        PolytopeIntersector& intersector;
        ArrayState& arrayState;
        const Polytope& polytope;
        ref_ptr<const vec3Array> sourceVertices;
        uint32_t instanceIndex = 0;

        std::vector<dvec3> processedVertices;
        std::vector<double> processedDistances;
        std::vector<dvec3> trimmedVertices;
        std::vector<double> trimmedDistances;

        PolytopePrimitiveIntersection(PolytopeIntersector& in_polyopeIntersector, ArrayState& in_arrayState, const Polytope& in_polytope) :
            intersector(in_polyopeIntersector),
            arrayState(in_arrayState),
            polytope(in_polytope)
        {
            size_t maxNumOfProcessedVertices = 3 + polytope.size();
            processedVertices.reserve(maxNumOfProcessedVertices);
            processedDistances.reserve(maxNumOfProcessedVertices);
            trimmedVertices.reserve(maxNumOfProcessedVertices);
            trimmedDistances.reserve(maxNumOfProcessedVertices);
        }

        bool instance(uint32_t index)
        {
            sourceVertices = arrayState.vertexArray(index);
            instanceIndex = index;
            return sourceVertices.valid();
        }

        void triangle(uint32_t i0, uint32_t i1, uint32_t i2)
        {
            // create a convex polygon from the 3 input vertices
            processedVertices.resize(3);
            processedVertices[0] = sourceVertices->at(i0);
            processedVertices[1] = sourceVertices->at(i1);
            processedVertices[2] = sourceVertices->at(i2);

            // trim the convex polygon to each successive plane
            for (const auto& pl : polytope)
            {
                size_t numNegativeDistances = 0;
                size_t numPositiveDistances = 0;
                size_t numZeroDistances = 0;
                processedDistances.resize(0);
                for (const auto& v : processedVertices)
                {
                    double d = distance(pl, v);
                    processedDistances.push_back(d);
                    if (d < 0.0)
                        ++numNegativeDistances;
                    else if (d > 0.0)
                        ++numPositiveDistances;
                    else
                        ++numZeroDistances;
                }

                if (numNegativeDistances > 0)
                {
                    if (numPositiveDistances == 0)
                    {
                        return; // wholly outside plane
                    }
                    for (size_t i = 0; i < processedVertices.size(); ++i)
                    {
                        size_t ni = (i + 1) % processedVertices.size();
                        if (processedDistances[i] >= 0.0)
                        {
                            trimmedVertices.push_back(processedVertices[i]);
                            trimmedDistances.push_back(processedDistances[i]);

                            if (processedDistances[ni] < 0.0) // i inside, ni outside
                            {
                                double r = processedDistances[i] / (processedDistances[i] - processedDistances[ni]);
                                dvec3 v = processedVertices[i] * (1.0 - r) + processedVertices[ni] * r;

                                trimmedVertices.push_back(v);
                                trimmedDistances.push_back(0.0);
                            }
                        }
                        else if (processedDistances[ni] > 0.0) // i inside, ni outside
                        {

                            double r = -processedDistances[i] / (processedDistances[ni] - processedDistances[i]);
                            dvec3 v = processedVertices[i] * (1.0 - r) + processedVertices[ni] * r;

                            trimmedVertices.push_back(v);
                            trimmedDistances.push_back(0.0);
                        }
                    }

                    // swap the newly trimmed with processed so they can be used in the new plane test
                    processedVertices.swap(trimmedVertices);
                    processedDistances.swap(trimmedDistances);

                    trimmedVertices.clear();
                    trimmedDistances.clear();

                    if (processedVertices.size() < 2)
                    {
                        return; // no triangle remaining inside plan
                    }
                }
            }

            dvec3 intersection(0.0, 0.0, 0.0);
            for (const auto& v : processedVertices)
            {
                intersection += v;
            }
            intersection /= static_cast<double>(processedVertices.size());

            intersector.add(intersection, {i0, i1, i2}, instanceIndex);
        }

        void line(uint32_t i0, uint32_t i1)
        {
            dvec3 v0(sourceVertices->at(i0));
            dvec3 v1(sourceVertices->at(i1));

            for (auto& pl : polytope)
            {
                double d0 = distance(pl, v0);
                double d1 = distance(pl, v1);
                if (d0 < 0.0)
                {
                    if (d1 < 0.0) return; // completely outside

                    // v0 outside, v1 inside
                    double r = -d0 / (d1 - d0);
                    v0 = v0 * (1.0 - r) + v1 * r;
                }
                else if (d1 < 0.0)
                {
                    // v0 inside, v2 outside
                    double r = -d1 / (d0 - d1);
                    v1 = v1 * (1.0 - r) + v0 * r;
                }
            }
            dvec3 intersection = (v0 + v1) * 0.5;
            intersector.add(intersection, {i0, i1}, instanceIndex);
        }

        void point(uint32_t i0)
        {
            const dvec3 v0(sourceVertices->at(i0));
            if (vsg::inside(polytope, v0))
            {
                intersector.add(v0, {i0}, instanceIndex);
            }
        }
    };

} // namespace vsg

PolytopeIntersector::PolytopeIntersector(const Polytope& in_polytope, ref_ptr<ArrayState> initialArrayData) :
    Inherit(initialArrayData)
{
    _polytopeStack.push_back(in_polytope);
}

PolytopeIntersector::PolytopeIntersector(const Camera& camera, double xMin, double yMin, double xMax, double yMax, ref_ptr<ArrayState> initialArrayData) :
    Inherit(initialArrayData)
{
    auto viewport = camera.getViewport();

    auto projectionMatrix = camera.projectionMatrix->transform();
    auto viewMatrix = camera.viewMatrix->transform();
    bool reverse_depth = (projectionMatrix(2, 2) > 0.0);

    double ndc_xMin = (viewport.width > 0) ? (2.0 * (xMin - static_cast<double>(viewport.x)) / static_cast<double>(viewport.width) - 1.0) : xMin;
    double ndc_xMax = (viewport.width > 0) ? (2.0 * (xMax - static_cast<double>(viewport.x)) / static_cast<double>(viewport.width) - 1.0) : xMax;

    double ndc_yMin = (viewport.height > 0) ? (2.0 * (yMin - static_cast<double>(viewport.y)) / static_cast<double>(viewport.height) - 1.0) : yMin;
    double ndc_yMax = (viewport.height > 0) ? (2.0 * (yMax - static_cast<double>(viewport.y)) / static_cast<double>(viewport.height) - 1.0) : yMax;

    double ndc_near = reverse_depth ? viewport.maxDepth : viewport.minDepth;
    double ndc_far = reverse_depth ? viewport.minDepth : viewport.maxDepth;

    vsg::Polytope clipspace;
    clipspace.push_back(dplane(1.0, 0.0, 0.0, -ndc_xMin)); // left
    clipspace.push_back(dplane(-1.0, 0.0, 0.0, ndc_xMax)); // right
    clipspace.push_back(dplane(0.0, 1.0, 0.0, -ndc_yMin)); // bottom
    clipspace.push_back(dplane(0.0, -1.0, 0.0, ndc_yMax)); // top
    clipspace.push_back(dplane(0.0, 0.0, -1.0, ndc_near)); // near
    clipspace.push_back(dplane(0.0, 0.0, 1.0, ndc_far));   // far

    vsg::Polytope eyespace;
    for (auto& pl : clipspace)
    {
        eyespace.push_back(pl * projectionMatrix);
    }

    _polytopeStack.push_back(eyespace);

    vsg::Polytope worldspace;
    for (auto& pl : eyespace)
    {
        worldspace.push_back(pl * viewMatrix);
    }

    _polytopeStack.push_back(worldspace);

    dmat4 eyeToWorld = inverse(viewMatrix);
    localToWorldStack().push_back(viewMatrix);
    worldToLocalStack().push_back(eyeToWorld);
}

PolytopeIntersector::Intersection::Intersection(const dvec3& in_localIntersection, const dvec3& in_worldIntersection, const dmat4& in_localToWorld, const NodePath& in_nodePath, const DataList& in_arrays, const std::vector<uint32_t>& in_indices, uint32_t in_instanceIndex) :
    localIntersection(in_localIntersection),
    worldIntersection(in_worldIntersection),
    localToWorld(in_localToWorld),
    nodePath(in_nodePath),
    arrays(in_arrays),
    indices(in_indices),
    instanceIndex(in_instanceIndex)
{
}

ref_ptr<PolytopeIntersector::Intersection> PolytopeIntersector::add(const dvec3& coord, const std::vector<uint32_t>& indices, uint32_t instanceIndex)
{
    ref_ptr<Intersection> intersection;

    auto localToWorld = computeTransform(_nodePath);
    intersection = Intersection::create(coord, localToWorld * coord, localToWorld, _nodePath, arrayStateStack.back()->arrays, indices, instanceIndex);
    intersections.emplace_back(intersection);

    return intersection;
}

void PolytopeIntersector::pushTransform(const Transform& transform)
{
    auto& l2wStack = localToWorldStack();
    auto& w2lStack = worldToLocalStack();

    dmat4 localToWorld = l2wStack.empty() ? transform.transform(dmat4{}) : transform.transform(l2wStack.back());
    dmat4 worldToLocal = inverse(localToWorld);

    l2wStack.push_back(localToWorld);
    w2lStack.push_back(worldToLocal);

    const auto& worldspace = _polytopeStack.front();

    Polytope localspace;
    for (auto& pl : worldspace)
    {
        localspace.push_back(pl * localToWorld);
    }

    _polytopeStack.push_back(localspace);
}

void PolytopeIntersector::popTransform()
{
    _polytopeStack.pop_back();
    localToWorldStack().pop_back();
    worldToLocalStack().pop_back();
}

bool PolytopeIntersector::intersects(const dsphere& bs)
{
    if (!bs.valid()) return false;

    const auto& polytope = _polytopeStack.back();

    return vsg::intersect(polytope, bs);
}

bool PolytopeIntersector::intersectDraw(uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount)
{
    size_t previous_size = intersections.size();

    auto& arrayState = *arrayStateStack.back();

    vsg::PrimitiveFunctor<vsg::PolytopePrimitiveIntersection> printPrimitives(*this, arrayState, _polytopeStack.back());
    printPrimitives.draw(arrayState.topology, firstVertex, vertexCount, firstInstance, instanceCount);

    return intersections.size() != previous_size;
}

bool PolytopeIntersector::intersectDrawIndexed(uint32_t firstIndex, uint32_t indexCount, uint32_t firstInstance, uint32_t instanceCount)
{
    size_t previous_size = intersections.size();

    auto& arrayState = *arrayStateStack.back();

    vsg::PrimitiveFunctor<vsg::PolytopePrimitiveIntersection> printPrimtives(*this, arrayState, _polytopeStack.back());
    if (ushort_indices)
        printPrimtives.drawIndexed(arrayState.topology, ushort_indices, firstIndex, indexCount, firstInstance, instanceCount);
    else if (uint_indices)
        printPrimtives.drawIndexed(arrayState.topology, uint_indices, firstIndex, indexCount, firstInstance, instanceCount);

    return intersections.size() != previous_size;
}
