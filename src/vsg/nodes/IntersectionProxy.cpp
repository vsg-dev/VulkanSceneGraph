/* <editor-fold desc="MIT License">

Copyright(c) 2025 Chris Djali

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/io/Logger.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/nodes/IntersectionProxy.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexDraw.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/utils/Intersector.h>
#include <vsg/utils/LineSegmentIntersector.h>

using namespace vsg;

namespace
{
    struct GetIndicesVisitor : public ConstVisitor
    {
        ref_ptr<const ubyteArray> ubyte_indices;
        ref_ptr<const ushortArray> ushort_indices;
        ref_ptr<const uintArray> uint_indices;

        void apply(const BufferInfo& bufferInfo) override
        {
            bufferInfo.data->accept(*this);
        }

        void apply(const ubyteArray& array) override
        {
            ubyte_indices = &array;
            ushort_indices = nullptr;
            uint_indices = nullptr;
        }
        void apply(const ushortArray& array) override
        {
            ubyte_indices = nullptr;
            ushort_indices = &array;
            uint_indices = nullptr;
        }
        void apply(const uintArray& array) override
        {
            ubyte_indices = nullptr;
            ushort_indices = nullptr;
            uint_indices = &array;
        }

        uint32_t operator[](size_t index)
        {
            if (ubyte_indices) return ubyte_indices->at(index);
            if (ushort_indices) return ushort_indices->at(index);
            return uint_indices->at(index);
        }
    };
}

IntersectionProxy::IntersectionProxy(Node* in_original) :
    Inherit(),
    original(in_original),
    internalNodes(),
    leaves(),
    bounds(),
    boundingVolumeHeirarchy({NodeRef::INVALID, 0u})
{
}

IntersectionProxy::IntersectionProxy(const IntersectionProxy& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    original(rhs.original),
    internalNodes(rhs.internalNodes),
    leaves(rhs.leaves),
    bounds(rhs.bounds),
    boundingVolumeHeirarchy(rhs.boundingVolumeHeirarchy)
{
}

void vsg::IntersectionProxy::rebuild(vsg::ArrayState& arrayState)
{
    leaves.clear();
    internalNodes.clear();

    if (!original)
    {
        warn("Attempting to build IntersectionProxy for null node.");
        return;
    }

    // if instancing is used, accessing the nth triangle is a hassle, so grab them upfront
    std::vector<Triangle> triangles;
    std::vector<TriangleMetadata> metadata;

    if (auto* vertexDraw = ::cast<VertexDraw>(original))
    {
        arrayState.apply(*vertexDraw);

        uint32_t lastIndex = vertexDraw->instanceCount > 1 ? (vertexDraw->firstInstance + vertexDraw->instanceCount) : vertexDraw->firstInstance + 1;
        uint32_t endVertex = vertexDraw->firstVertex + vertexDraw->vertexCount;

        triangles.reserve(vertexDraw->instanceCount * vertexDraw->vertexCount / 3);
        metadata.reserve(triangles.size());

        for (uint32_t instanceIndex = vertexDraw->firstInstance; instanceIndex < lastIndex; ++instanceIndex)
        {
            if (auto vertices = arrayState.vertexArray(instanceIndex))
            {
                for (uint32_t i = vertexDraw->firstVertex; (i + 2) < endVertex; i += 3)
                {
                    triangles.emplace_back(Triangle{vertices->at(i), vertices->at(i + 1), vertices->at(i + 2)});
                    metadata.emplace_back(TriangleMetadata{i, i + 1, i + 2, instanceIndex});
                }
            }
        }
    }
    else if (auto* vertexIndexDraw = ::cast<VertexIndexDraw>(original))
    {
        arrayState.apply(*vertexIndexDraw);

        uint32_t lastIndex = vertexIndexDraw->instanceCount > 1 ? (vertexIndexDraw->firstInstance + vertexIndexDraw->instanceCount) : vertexIndexDraw->firstInstance + 1;
        uint32_t endIndex = vertexIndexDraw->firstIndex + ((vertexIndexDraw->indexCount + 2) / 3) * 3;

        triangles.reserve(vertexIndexDraw->instanceCount * vertexIndexDraw->indexCount / 3);
        metadata.reserve(triangles.size());

        if (!vertexIndexDraw->indices || !vertexIndexDraw->indices->data)
        {
            warn("Attempting to build IntersectionProxy for VertexIndexDraw with no indices.");
            return;
        }

        GetIndicesVisitor indices;
        vertexIndexDraw->indices->accept(indices);

        for (uint32_t instanceIndex = vertexIndexDraw->firstInstance; instanceIndex < lastIndex; ++instanceIndex)
        {
            if (auto vertices = arrayState.vertexArray(instanceIndex))
            {
                for (uint32_t i = vertexIndexDraw->firstIndex; i < endIndex; i += 3)
                {
                    triangles.emplace_back(Triangle{vertices->at(indices[i]), vertices->at(indices[i + 1]), vertices->at(indices[i + 2])});
                    metadata.emplace_back(TriangleMetadata{indices[i], indices[i + 1], indices[i + 2], instanceIndex});
                }
            }
        }
    }
    else
    {
        warn("Unsupported node type when building IntersectionProxy: ", original->className());
        return;
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

bool IntersectionProxy::valid() const
{
    return boundingVolumeHeirarchy.type != NodeRef::INVALID;
}

void vsg::IntersectionProxy::intersect(LineSegmentIntersector& lineSegmentIntersector) const
{
    const auto& ls = lineSegmentIntersector.lineSegment();

    using value_type = double;
    using vec_type = t_vec3<value_type>;
    const value_type epsilon = 1e-10;

    vec_type start(ls.start);
    vec_type end(ls.end);

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

    // vec_type dInvX = d.x != 0.0 ? d / d.x : vec_type{0.0, 0.0, 0.0};
    // vec_type dInvY = d.y != 0.0 ? d / d.y : vec_type{0.0, 0.0, 0.0};
    // vec_type dInvZ = d.z != 0.0 ? d / d.z : vec_type{0.0, 0.0, 0.0};

    auto intersectLeaf = [&](uint32_t index) {
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

            value_type r0 = 1.0 - u - v;
            value_type r1 = u;
            value_type r2 = v;

            dvec3 intersection = dvec3(triangle.vertex0) * double(r0) + dvec3(triangle.vertex1) * double(r1) + dvec3(triangle.vertex2) * double(r2);
            const auto& metadata = leafMetadata[index].tris[i];
            lineSegmentIntersector.add(intersection, double(t * inverseLength), {{metadata.index0, r0}, {metadata.index1 + 1, r1}, {metadata.index2 + 2, r2}}, metadata.instance);
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

IntersectionProxy::~IntersectionProxy() = default;

int IntersectionProxy::compare(const Object& rhs_object) const
{
    int result = Node::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_pointer(original, rhs.original)) != 0) return result;
    // computation of BVH should be deterministic, so if the input is the same and it's actually been computed, we shouldn't need to scan all the nodes
    if ((result = compare_value(boundingVolumeHeirarchy.type, rhs.boundingVolumeHeirarchy.type)) != 0) return result;
    return compare_value(boundingVolumeHeirarchy.index, boundingVolumeHeirarchy.index);
}

void IntersectionProxy::read(Input& input)
{
    Node::read(input);

    input.read("original", original);
    // todo: deserialise BVH
}

void IntersectionProxy::write(Output& output) const
{
    Node::write(output);

    output.write("original", original);
    // todo: serialise BVH
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
            auto optimized = vsg::IntersectionProxy::create(child);
            optimized->rebuild(*arrayStateStack.back());
            child = optimized;
        }
    }

    arrayStateStack.pop_back();
}
