/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Draw.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/StateGroup.h>
#include <vsg/traversals/Intersector.h>

using namespace vsg;

struct PushPopNode
{
    Intersector::NodePath& nodePath;

    PushPopNode(Intersector::NodePath& np, const Node* node) :
        nodePath(np) { nodePath.push_back(node); }
    ~PushPopNode() { nodePath.pop_back(); }
};

Intersector::Intersector()
{
    _topologyStack.push_back(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
}

void Intersector::apply(const Node& node)
{
    PushPopNode ppn(_nodePath, &node);

    node.traverse(*this);
}

void Intersector::apply(const StateGroup& stategroup)
{
    PushPopNode ppn(_nodePath, &stategroup);

    // currently just tracking InputArrayState::topology
    // TODO : review if we need to track and handle more parameters and handle vertex shaders computing vertex positions other than just using vertex and project and modelview matrix.
    auto previous_topology = topology();

    struct FindGraphicsPipelineVisitor : public ConstVisitor
    {
        VkPrimitiveTopology topology;
        Intersector::AttributeDetails vertexAttribute;
        uint32_t vertex_attribute_location = 0;

        FindGraphicsPipelineVisitor(VkPrimitiveTopology in_topology) :
            topology(in_topology) {}

        void apply(const BindGraphicsPipeline& bpg) override
        {
            for (auto& pipelineState : bpg.getPipeline()->getPipelineStates())
            {
                if (auto ias = pipelineState.cast<InputAssemblyState>(); ias)
                    topology = ias->topology;
                else if (auto vas = pipelineState.cast<VertexInputState>(); vas)
                {
                    for (auto& attribute : vas->getAttributes())
                    {
                        if (attribute.location == vertex_attribute_location)
                        {
                            for (auto& binding : vas->geBindings())
                            {
                                if (attribute.binding == binding.binding)
                                {
                                    vertexAttribute.binding = attribute.binding;
                                    vertexAttribute.offset = attribute.offset;
                                    vertexAttribute.stride = binding.stride;
                                    vertexAttribute.format = attribute.format;
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    } findGraphicsPipeline(previous_topology);

    for (auto& state : stategroup.getStateCommands())
    {
        state->accept(findGraphicsPipeline);
    }

    if (findGraphicsPipeline.topology != previous_topology)
    {
        _topologyStack.push_back(findGraphicsPipeline.topology);
    }

    if (findGraphicsPipeline.vertexAttribute.stride != 0)
    {
        AttributeDetails previous_vertexAttribute = vertexAttribute;

        vertexAttribute = findGraphicsPipeline.vertexAttribute;
        stategroup.traverse(*this);

        vertexAttribute = previous_vertexAttribute;
    }
    else
    {
        stategroup.traverse(*this);
    }

    if (findGraphicsPipeline.topology != previous_topology)
    {
        _topologyStack.pop_back();
    }
}

void Intersector::apply(const MatrixTransform& transform)
{
    PushPopNode ppn(_nodePath, &transform);

    pushTransform(transform.getMatrix());

    transform.traverse(*this);

    popTransform();
}

void Intersector::apply(const LOD& lod)
{
    PushPopNode ppn(_nodePath, &lod);

    if (intersects(lod.getBound()))
    {
        for (auto& child : lod.getChildren())
        {
            if (child.node)
            {
                child.node->accept(*this);
                break;
            }
        }
    }
}

void Intersector::apply(const PagedLOD& plod)
{
    PushPopNode ppn(_nodePath, &plod);

    if (intersects(plod.getBound()))
    {
        for (auto& child : plod.getChildren())
        {
            if (child.node)
            {
                child.node->accept(*this);
                break;
            }
        }
    }
}

void Intersector::apply(const CullNode& cn)
{
    PushPopNode ppn(_nodePath, &cn);

    if (intersects(cn.getBound())) cn.traverse(*this);
}

void Intersector::apply(const VertexIndexDraw& vid)
{
    apply(vid.firstBinding, vid.arrays);
    if (!_vertices) return;

    PushPopNode ppn(_nodePath, &vid);

    sphere bound;
    if (!vid.getValue("bound", bound))
    {
        box bb;
        for (auto& vertex : *_vertices) bb.add(vertex);

        if (bb.valid())
        {
            bound.center = (bb.min + bb.max) * 0.5f;
            bound.radius = length(bb.max - bb.min) * 0.5f;

            // hacky but better to reuse results.  Perhaps use a std::map<> to avoid a breaking const, or make the vistitor non const?
            const_cast<VertexIndexDraw&>(vid).setValue("bound", bound);
        }

        // std::cout<<"Computed bounding sphere : "<<bound.center<<", "<<bound.radius<<std::endl;
    }
    else
    {
        // std::cout<<"Found bounding sphere : "<<bound.center<<", "<<bound.radius<<std::endl;
    }

    if (intersects(bound))
    {
        intersect(topology(), _vertices, vid.indices, vid.firstIndex, vid.indexCount);
    }
}

void Intersector::apply(const Geometry& geometry)
{
    apply(geometry.firstBinding, geometry.arrays);
    if (!_vertices) return;

    PushPopNode ppn(_nodePath, &geometry);

    _indices = geometry.indices;

    for (auto& command : geometry.commands)
    {
        command->accept(*this);
    }
}

void Intersector::apply(const BindVertexBuffers& bvb)
{
    apply(bvb.getFirstBinding(), bvb.getArrays());
}

void Intersector::apply(const BindIndexBuffer& bib)
{
    _indices = bib.getIndices();
}

void Intersector::apply(uint32_t firstBinding, const DataList& arrays)
{
    if ((vertexAttribute.binding >= firstBinding) && ((vertexAttribute.binding - firstBinding) < arrays.size()) && (vertexAttribute.format == VK_FORMAT_R32G32B32_SFLOAT))
    {
        auto array = arrays[vertexAttribute.binding - firstBinding];
        _vertices = array.cast<vec3Array>();
        if (!_vertices && vertexAttribute.stride > 0)
        {
            if (!proxy_vertexArray) proxy_vertexArray = vsg::vec3Array::create();

            uint32_t numVertices = array->dataSize() / vertexAttribute.stride;
            proxy_vertexArray->assign(array, vertexAttribute.offset, vertexAttribute.stride, numVertices, array->getLayout());

            _vertices = proxy_vertexArray;
        }
    }
    else
    {
        _vertices = nullptr;
    }

    _arrays = arrays;
}

void Intersector::apply(const Draw& draw)
{
    if (!_vertices) return;

    PushPopNode ppn(_nodePath, &draw);

    intersect(topology(), _vertices, draw.firstVertex, draw.vertexCount);
}

void Intersector::apply(const DrawIndexed& drawIndexed)
{
    if (!_vertices) return;

    PushPopNode ppn(_nodePath, &drawIndexed);

    intersect(topology(), _vertices, _indices, drawIndexed.firstIndex, drawIndexed.indexCount);
}
