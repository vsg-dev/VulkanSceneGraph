/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Commands.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/StateGroup.h>
#include <vsg/traversals/ComputeBounds.h>

using namespace vsg;

ComputeBounds::ComputeBounds()
{
}

void ComputeBounds::apply(const vsg::Node& node)
{
    node.traverse(*this);
}

void ComputeBounds::apply(const StateGroup& stategroup)
{
    struct FindGraphicsPipelineVisitor : public ConstVisitor
    {
        ComputeBounds::AttributeDetails vertexAttribute;
        uint32_t vertex_attribute_location = 0;

        FindGraphicsPipelineVisitor() {}

        void apply(const BindGraphicsPipeline& bpg) override
        {
            for (auto& pipelineState : bpg.getPipeline()->getPipelineStates())
            {
                if (auto vas = pipelineState.cast<VertexInputState>(); vas)
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
    } findGraphicsPipeline;

    for (auto& state : stategroup.getStateCommands())
    {
        state->accept(findGraphicsPipeline);
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
}

void ComputeBounds::apply(const vsg::MatrixTransform& transform)
{
    matrixStack.push_back(transform.getMatrix());

    transform.traverse(*this);

    matrixStack.pop_back();
}

void ComputeBounds::apply(const vsg::Geometry& geometry)
{
    apply(geometry.firstBinding, geometry.arrays);
}

void ComputeBounds::apply(const vsg::VertexIndexDraw& vid)
{
    apply(vid.firstBinding, vid.arrays);
}

void ComputeBounds::apply(const vsg::BindVertexBuffers& bvb)
{
    apply(bvb.getFirstBinding(), bvb.getArrays());
}

void ComputeBounds::apply(uint32_t firstBinding, const DataList& arrays)
{
    if ((vertexAttribute.binding >= firstBinding) && ((vertexAttribute.binding - firstBinding) < arrays.size()) && (vertexAttribute.format == VK_FORMAT_R32G32B32_SFLOAT))
    {
        auto array = arrays[vertexAttribute.binding - firstBinding];
        auto vertices = array.cast<vec3Array>();
        if (vertices)
        {
            apply(*vertices);
        }
        else if (vertexAttribute.stride > 0)
        {
            if (!proxy_vertexArray) proxy_vertexArray = vsg::vec3Array::create();

            uint32_t numVertices = array->dataSize() / vertexAttribute.stride;
            proxy_vertexArray->assign(array, vertexAttribute.offset, vertexAttribute.stride, numVertices, array->getLayout());

            proxy_vertexArray->accept(*this);
        }
    }
}

void ComputeBounds::apply(const vsg::vec3Array& vertices)
{
    if (matrixStack.empty())
    {
        for (auto vertex : vertices) bounds.add(vertex);
    }
    else
    {
        auto matrix = matrixStack.back();
        for (auto vertex : vertices) bounds.add(matrix * vertex);
    }
}
