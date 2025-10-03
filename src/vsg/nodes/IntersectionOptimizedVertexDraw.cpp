/* <editor-fold desc="MIT License">

Copyright(c) 2025 Chris Djali

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/io/Logger.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/nodes/IntersectionOptimizedVertexDraw.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/utils/Intersector.h>
#include <vsg/utils/LineSegmentIntersector.h>

using namespace vsg;

IntersectionOptimizedVertexDraw::IntersectionOptimizedVertexDraw(const VertexDraw& vertexDraw, const CopyOp& copyop) :
    Inherit(vertexDraw, copyop),
    internalNodes(),
    leaves(),
    bounds(),
    boundingVolumeHeirarchy({NodeRef::INVALID, 0u})
{
}

IntersectionOptimizedVertexDraw::IntersectionOptimizedVertexDraw(const IntersectionOptimizedVertexDraw& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    internalNodes(rhs.internalNodes),
    leaves(rhs.leaves),
    bounds(rhs.bounds),
    boundingVolumeHeirarchy(rhs.boundingVolumeHeirarchy)
{
}

void vsg::IntersectionOptimizedVertexDraw::rebuild(vsg::ArrayState& arrayState)
{
    leaves.clear();
    internalNodes.clear();

    arrayState.apply(*this);
    uint32_t lastIndex = instanceCount > 1 ? (firstInstance + instanceCount) : firstInstance + 1;
    uint32_t endVertex = firstVertex + vertexCount;

    // if instancing is used, accessing the nth triangle is a hassle, so grab them upfront
    std::vector<Triangle> triangles;
    triangles.reserve(instanceCount * vertexCount / 3);
    std::vector<TriangleMetadata> metadata;
    metadata.reserve(triangles.size());

    for (uint32_t instanceIndex = firstInstance; instanceIndex < lastIndex; ++instanceIndex)
    {
        if (auto vertices = arrayState.vertexArray(instanceIndex))
        {
            for (uint32_t i = firstVertex; (i + 2) < endVertex; i += 3)
            {
                triangles.emplace_back(Triangle{vertices->at(i), vertices->at(i + 1), vertices->at(i + 2)});
                metadata.emplace_back(TriangleMetadata{i, instanceIndex});
            }
        }
    }

    std::vector<vec3> barycenters;
    barycenters.reserve(triangles.size());
    for (const auto& triangle : triangles)
    {
        barycenters.emplace_back((triangle.vertex0 + triangle.vertex1 + triangle.vertex2) / 3.f);
    }

    std::vector<size_t> indices;
    indices.reserve(triangles.size());
    for (size_t i = 0; i < triangles.size(); ++i)
    {
        indices.emplace_back(i);
    }

    leaves.reserve(triangles.size() / trisPerLeaf);
    internalNodes.reserve(triangles.size() / (trisPerLeaf * 2));

    using itr_t = decltype(indices)::iterator;

    auto computeKDTree = [&](itr_t first, itr_t last, auto&& computeKDTree) -> std::pair<box, NodeRef> {
        if (std::distance(first, last) < trisPerLeaf)
        {
            leaves.emplace_back();
            leafMetadata.emplace_back();
            box bound;
            itr_t itr = first;
            for (size_t i = 0; i < trisPerLeaf; ++i)
            {
                if (itr != last)
                {
                    leaves.back().tris[i] = triangles[*itr];
                    bound.add(triangles[*itr].vertex0);
                    bound.add(triangles[*itr].vertex1);
                    bound.add(triangles[*itr].vertex2);
                    leafMetadata.back().tris[i] = metadata[*itr];
                    ++itr;
                }
                else
                {
                    // add a degenerate triangle as we have to have trisPerLeaf
                    leaves.back().tris[i] = {vec3(), vec3(), vec3()};
                }
            }
            return std::make_pair(bound, NodeRef{NodeRef::LEAF, static_cast<uint32_t>(leaves.size() - 1)});
        }
        else
        {
            box baryBound;
            for (itr_t itr = first; itr != last; ++itr)
            {
                baryBound.add(barycenters[*itr]);
            }
            vec3 range = baryBound.max - baryBound.min;
            size_t axisIndex = range.x > range.y ? (range.x > range.z ? 0 : 2) : (range.y > range.z ? 1 : 2);
            itr_t midpoint = first + std::distance(first, last) / 2;
            std::nth_element(first, midpoint, last, [&, axisIndex](const size_t& lhs, const size_t& rhs) { return barycenters[lhs][axisIndex] < barycenters[rhs][axisIndex]; });
            internalNodes.emplace_back(InternalNode{{computeKDTree(first, midpoint, computeKDTree), computeKDTree(midpoint, last, computeKDTree)}});
            box overallBound;
            for (const auto& [bound, ref] : internalNodes.back().children)
            {
                overallBound.add(bound);
            }
            return std::make_pair(overallBound, NodeRef{NodeRef::INTERNAL, static_cast<uint32_t>(internalNodes.size() - 1)});
        }
    };

    std::tie(bounds, boundingVolumeHeirarchy) = computeKDTree(indices.begin(), indices.end(), computeKDTree);
}

void vsg::IntersectionOptimizedVertexDraw::intersect(LineSegmentIntersector& lineSegmentIntersector) const
{
    const auto& ls = lineSegmentIntersector.lineSegment();

    using value_type = double;
    using vec_type = t_vec3<value_type>;
    const value_type epsilon = 1e-10;

    vec_type start = ls.start;
    vec_type end = ls.end;

    vec_type d = end - start;

    auto intersectBox = [&](const box& bound) {
        value_type t1 = (bound.min.x - start.x) / d.x;
        value_type t2 = (bound.max.x - start.x) / d.x;
        value_type tmin = std::min(t1, t2);
        value_type tmax = std::max(t1, t2);
        t1 = (bound.min.y - start.y) / d.y;
        t2 = (bound.max.y - start.y) / d.y;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
        t1 = (bound.min.z - start.z) / d.z;
        t2 = (bound.max.z - start.z) / d.z;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
        return tmax >= tmin && tmin < 1.0 && tmax > 0.0;
    };

    if (!bounds.valid() || !intersectBox(bounds))
        return;

    value_type length = ::length(d);
    value_type inverseLength = length != 0.0 ? 1.0 / length : 0.0;

    vec_type dInvX = d.x != 0.0 ? d / d.x : vec_type{0.0, 0.0, 0.0};
    vec_type dInvY = d.y != 0.0 ? d / d.y : vec_type{0.0, 0.0, 0.0};
    vec_type dInvZ = d.z != 0.0 ? d / d.z : vec_type{0.0, 0.0, 0.0};

    auto intersectLeaf = [&](uint32_t index) {
        // using different but consecutive addresses for different triangle results should encourage vectorisation
        std::array<value_type, trisPerLeaf> r, r0, r1, r2;
        r.fill(std::numeric_limits<value_type>::quiet_NaN());
        for (size_t i = 0; i < trisPerLeaf; ++i)
        {
            const auto& triangle = leaves[index].tris[i];

            vec_type E1 = vec_type(triangle.vertex1) - vec_type(triangle.vertex0);
            vec_type E2 = vec_type(triangle.vertex2) - vec_type(triangle.vertex0);

            vec_type P = cross(d, E2);
            value_type det = dot(P, E1);
            if (det > -epsilon && det < epsilon) continue;

            value_type inv_det = 1.0 / det;
            vec_type T = vec_type(start) - vec_type(triangle.vertex0);
            value_type u = inv_det * dot(P, T);
            if (u < 0.0 || u > 1) continue;

            vec_type Q = cross(T, E1);
            value_type v = inv_det * dot(Q, d);
            if (v < 0.0 || u + v > 1.0) continue;

            value_type t = inv_det * dot(Q, E2);
            if (t < epsilon) continue;
            
            r0[i] = 1.0 - u - v;
            r1[i] = u;
            r2[i] = v;
            r[i] = t * inverseLength;
        }

        // this loop is separate because the intersector modification won't vectorise
        for (size_t i = 0; i < trisPerLeaf; ++i)
        {
            if (std::isnan(r[i])) continue;

            const auto& triangle = leaves[index].tris[i];
            dvec3 intersection = dvec3(triangle.vertex0) * double(r0[i]) + dvec3(triangle.vertex1) * double(r1[i]) + dvec3(triangle.vertex2) * double(r2[i]);
            const auto& metadata = leafMetadata[index].tris[i];
            lineSegmentIntersector.add(intersection, double(r[i]), {{metadata.index, r0[i]}, {metadata.index + 1, r1[i]}, {metadata.index + 2, r2[i]}}, metadata.instance);
        }
    };

    auto intersectNode = [&](const NodeRef& nodeRef, auto&& intersectNode) -> void {
        if (nodeRef.type == NodeRef::LEAF)
        {
            intersectLeaf(nodeRef.index);
        }
        else
        {
            const auto& node = internalNodes[nodeRef.index];
            for (const auto& [bound, child] : node.children)
            {
                if (intersectBox(bound))
                {
                    intersectNode(child, intersectNode);
                }
            }
        }
    };

    intersectNode(boundingVolumeHeirarchy, intersectNode);
}

IntersectionOptimizedVertexDraw::~IntersectionOptimizedVertexDraw() = default;

int IntersectionOptimizedVertexDraw::compare(const Object& rhs_object) const
{
    int result = VertexDraw::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    // computation of BVH should be deterministic, so if the input is the same and it's actually been computed, we shouldn't need to scan all the nodes
    if ((result = compare_value(boundingVolumeHeirarchy.type, rhs.boundingVolumeHeirarchy.type)) != 0) return result;
    return compare_value(boundingVolumeHeirarchy.index, boundingVolumeHeirarchy.index);
}

void IntersectionOptimizedVertexDraw::read(Input& input)
{
    VertexDraw::read(input);
}

void IntersectionOptimizedVertexDraw::write(Output& output) const
{
    VertexDraw::write(output);
}

void vsg::IntersectionOptimizedVertexDraw::accept(ConstVisitor& visitor) const
{
    if (boundingVolumeHeirarchy.type != NodeRef::INVALID && visitor.is_compatible(typeid(Intersector)))
    {
        auto* lineSegmentIntersector = dynamic_cast<LineSegmentIntersector*>(&visitor);
        if (lineSegmentIntersector)
        {
            intersect(*lineSegmentIntersector);
            return;
        }
    }
    Inherit::accept(visitor);
}

vsg::IntersectionOptimizeVisitor::IntersectionOptimizeVisitor(ref_ptr<ArrayState> initialArrayState)
{
    arrayStateStack.reserve(4);
    arrayStateStack.emplace_back(initialArrayState ? initialArrayState : ArrayState::create());
}

void vsg::IntersectionOptimizeVisitor::apply(Node& node)
{
    node.traverse(*this);
}

void vsg::IntersectionOptimizeVisitor::apply(StateGroup& stategroup)
{
    auto arrayState = stategroup.prototypeArrayState ? stategroup.prototypeArrayState->cloneArrayState(arrayStateStack.back()) : arrayStateStack.back()->cloneArrayState();

    for (auto& statecommand : stategroup.stateCommands)
    {
        statecommand->accept(*arrayState);
    }

    arrayStateStack.emplace_back(arrayState);

    stategroup.traverse(*this);

    for (auto& child : stategroup.children)
    {
        if (child->className() == "vsg::VertexDraw")
        {
            auto optimized = vsg::IntersectionOptimizedVertexDraw::create(static_cast<VertexDraw&>(*child));
            optimized->rebuild(*arrayStateStack.back());
            child = optimized;
        }
    }

    arrayStateStack.pop_back();
}
