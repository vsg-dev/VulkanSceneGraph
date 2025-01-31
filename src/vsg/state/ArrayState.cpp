/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/maths/sample.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/VertexDraw.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/ArrayState.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/VertexInputState.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ArrayState
//
ArrayState::ArrayState(const ArrayState& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    localToWorldStack(rhs.localToWorldStack),
    worldToLocalStack(rhs.worldToLocalStack),
    topology(rhs.topology),
    vertex_attribute_location(rhs.vertex_attribute_location),
    vertexAttribute(rhs.vertexAttribute),
    vertices(rhs.vertices),
    proxy_vertices(rhs.proxy_vertices),
    arrays(rhs.arrays)
{
}

ref_ptr<const vec3Array> ArrayState::vertexArray(uint32_t /*instanceIndex*/)
{
    return vertices;
}

void ArrayState::apply(const vsg::BindGraphicsPipeline& bpg)
{
    for (auto& pipelineState : bpg.pipeline->pipelineStates)
    {
        pipelineState->accept(*this);
    }
}

bool ArrayState::getAttributeDetails(const VertexInputState& vas, uint32_t location, AttributeDetails& attributeDetails)
{
    for (const auto& attribute : vas.vertexAttributeDescriptions)
    {
        if (attribute.location == location)
        {
            for (const auto& binding : vas.vertexBindingDescriptions)
            {
                if (attribute.binding == binding.binding)
                {
                    attributeDetails.binding = attribute.binding;
                    attributeDetails.format = attribute.format;
                    attributeDetails.offset = attribute.offset;
                    attributeDetails.stride = binding.stride;
                    attributeDetails.inputRate = binding.inputRate;
                    return true;
                }
            }
        }
    }
    return false;
}

void ArrayState::apply(const VertexInputState& vas)
{
    getAttributeDetails(vas, vertex_attribute_location, vertexAttribute);
}

void ArrayState::apply(const InputAssemblyState& ias)
{
    topology = ias.topology;
}

void ArrayState::apply(const vsg::Geometry& geometry)
{
    applyArrays(geometry.firstBinding, geometry.arrays);
}

void ArrayState::apply(const vsg::VertexDraw& vid)
{
    applyArrays(vid.firstBinding, vid.arrays);
}

void ArrayState::apply(const vsg::VertexIndexDraw& vid)
{
    applyArrays(vid.firstBinding, vid.arrays);
}

void ArrayState::apply(const vsg::BindVertexBuffers& bvb)
{
    applyArrays(bvb.firstBinding, bvb.arrays);
}

void ArrayState::applyArrays(uint32_t firstBinding, const DataList& in_arrays)
{
    if (arrays.size() < (in_arrays.size() + firstBinding)) arrays.resize(in_arrays.size() + firstBinding);
    std::copy(in_arrays.begin(), in_arrays.end(), arrays.begin() + firstBinding);

    // if the required vertexAttribute is within the new arrays apply the appropriate array to set up the vertices array
    if ((vertexAttribute.binding >= firstBinding) && ((vertexAttribute.binding - firstBinding) < arrays.size()) && arrays[vertexAttribute.binding])
    {
        arrays[vertexAttribute.binding]->accept(*this);
    }
}

void ArrayState::applyArrays(uint32_t firstBinding, const BufferInfoList& in_arrays)
{
    if (arrays.size() < (in_arrays.size() + firstBinding)) arrays.resize(in_arrays.size() + firstBinding);
    for (size_t i = 0; i < in_arrays.size(); ++i)
    {
        arrays[firstBinding + i] = in_arrays[i]->data;
    }

    // if the required vertexAttribute is within the new arrays apply the appropriate array to set up the vertices array
    if ((vertexAttribute.binding >= firstBinding) && ((vertexAttribute.binding - firstBinding) < arrays.size()) && arrays[vertexAttribute.binding])
    {
        arrays[vertexAttribute.binding]->accept(*this);
    }
}

void ArrayState::apply(const vsg::BufferInfo& bufferInfo)
{
    if (bufferInfo.data) bufferInfo.data->accept(*this);
}

void ArrayState::apply(const vsg::vec3Array& array)
{
    vertices = &array;
}

void ArrayState::apply(const vsg::Data& array)
{
    // array hasn't been matched to vec3Array so fallback to using a proxy array to adapt it
    if (vertexAttribute.stride > 0 && (vertexAttribute.format == VK_FORMAT_R32G32B32_SFLOAT))
    {
        if (!proxy_vertices) proxy_vertices = vsg::vec3Array::create();

        uint32_t numVertices = static_cast<uint32_t>(arrays[vertexAttribute.binding]->dataSize()) / vertexAttribute.stride;
        proxy_vertices->assign(arrays[vertexAttribute.binding], vertexAttribute.offset, vertexAttribute.stride, numVertices, array.properties);

        vertices = proxy_vertices;
    }
    else
    {
        vertices = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// NullArrayState
//
NullArrayState::NullArrayState() :
    Inherit()
{
}

NullArrayState::NullArrayState(const ArrayState& as) :
    Inherit(as)
{
    vertices = {};
}

ref_ptr<ArrayState> NullArrayState::cloneArrayState()
{
    return NullArrayState::create(*this);
}

// clone the specified ArrayState
ref_ptr<ArrayState> NullArrayState::cloneArrayState(ref_ptr<ArrayState> arrayState)
{
    return NullArrayState::create(*arrayState);
}

void NullArrayState::apply(const vsg::vec3Array&)
{
    vertices = {};
}

void NullArrayState::apply(const vsg::Data&)
{
    vertices = {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PositionArrayState
//
PositionArrayState::PositionArrayState()
{
}

PositionArrayState::PositionArrayState(const PositionArrayState& rhs) :
    Inherit(rhs),
    position_attribute_location(rhs.position_attribute_location),
    positionAttribute(rhs.positionAttribute)
{
}

PositionArrayState::PositionArrayState(const ArrayState& rhs) :
    Inherit(rhs)
{
}

ref_ptr<ArrayState> PositionArrayState::cloneArrayState()
{
    return PositionArrayState::create(*this);
}

ref_ptr<ArrayState> PositionArrayState::cloneArrayState(ref_ptr<ArrayState> arrayState)
{
    return PositionArrayState::create(*arrayState);
}

void PositionArrayState::apply(const VertexInputState& vas)
{
    getAttributeDetails(vas, vertex_attribute_location, vertexAttribute);
    getAttributeDetails(vas, position_attribute_location, positionAttribute);
}

ref_ptr<const vec3Array> PositionArrayState::vertexArray(uint32_t instanceIndex)
{
    auto positions = arrays[positionAttribute.binding].cast<vec3Array>();

    if (positions && (instanceIndex < positions->size()))
    {
        auto position = positions->at(instanceIndex);
        auto new_vertices = vsg::vec3Array::create(static_cast<uint32_t>(vertices->size()));
        auto src_vertex_itr = vertices->begin();
        for (auto& v : *new_vertices)
        {
            v = *(src_vertex_itr++) + position;
        }
        return new_vertices;
    }

    return vertices;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DisplacementMapArrayState
//
DisplacementMapArrayState::DisplacementMapArrayState()
{
}

DisplacementMapArrayState::DisplacementMapArrayState(const DisplacementMapArrayState& rhs) :
    Inherit(rhs)
{
}

DisplacementMapArrayState::DisplacementMapArrayState(const ArrayState& rhs) :
    Inherit(rhs)
{
}

ref_ptr<ArrayState> DisplacementMapArrayState::cloneArrayState()
{
    return DisplacementMapArrayState::create(*this);
}

ref_ptr<ArrayState> DisplacementMapArrayState::cloneArrayState(ref_ptr<ArrayState> arrayState)
{
    return DisplacementMapArrayState::create(*arrayState);
}

void DisplacementMapArrayState::apply(const DescriptorImage& di)
{
    if (!di.imageInfoList.empty())
    {
        const auto& imageInfo = *di.imageInfoList[0];
        if (imageInfo.imageView && imageInfo.imageView->image)
        {
            displacementMap = imageInfo.imageView->image->data.cast<floatArray2D>();
            sampler = imageInfo.sampler;
        }
    }
}

void DisplacementMapArrayState::apply(const DescriptorSet& ds)
{
    for (auto& descriptor : ds.descriptors)
    {
        if (descriptor->dstBinding == dm_binding)
        {
            descriptor->accept(*this);
            break;
        }
    }
}

void DisplacementMapArrayState::apply(const BindDescriptorSet& bds)
{
    if (bds.firstSet == dm_set)
    {
        apply(*bds.descriptorSet);
    }
}

void DisplacementMapArrayState::apply(const BindDescriptorSets& bds)
{
    if (bds.firstSet <= dm_set && dm_set < (bds.firstSet + +static_cast<uint32_t>(bds.descriptorSets.size())))
    {
        apply(*bds.descriptorSets[dm_set - bds.firstSet]);
    }
}

void DisplacementMapArrayState::apply(const VertexInputState& vas)
{
    getAttributeDetails(vas, vertex_attribute_location, vertexAttribute);
    getAttributeDetails(vas, normal_attribute_location, normalAttribute);
    getAttributeDetails(vas, texcoord_attribute_location, texcoordAttribute);
}

ref_ptr<const vec3Array> DisplacementMapArrayState::vertexArray(uint32_t /*instanceIndex*/)
{
    if (displacementMap)
    {
        auto normals = arrays[normalAttribute.binding].cast<vec3Array>();
        auto texcoords = arrays[texcoordAttribute.binding].cast<vec2Array>();
        if (texcoords->size() != vertices->size()) return {};
        if (normals->size() != vertices->size()) return {};

        auto new_vertices = vsg::vec3Array::create(static_cast<uint32_t>(vertices->size()));
        auto src_vertex_itr = vertices->begin();
        auto src_texcoord_itr = texcoords->begin();
        auto src_normal_itr = normals->begin();
        // vec2 tc_scale(static_cast<float>(displacementMap->width()) - 1.0f, static_cast<float>(displacementMap->height()) - 1.0f);

        // if no sampler is assigned fallback to use default constructed Sampler
        if (!sampler) sampler = Sampler::create();

        for (auto& v : *new_vertices)
        {
            const auto& tc = *(src_texcoord_itr++);
            const auto& n = *(src_normal_itr++);
            float d = sample(*sampler, *displacementMap, tc);
            v = *(src_vertex_itr++) + n * d;
        }
        return new_vertices;
    }

    return vertices;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PositionAndDisplacementMapArrayState
//
PositionAndDisplacementMapArrayState::PositionAndDisplacementMapArrayState()
{
}

PositionAndDisplacementMapArrayState::PositionAndDisplacementMapArrayState(const PositionAndDisplacementMapArrayState& rhs) :
    Inherit(rhs)
{
}

PositionAndDisplacementMapArrayState::PositionAndDisplacementMapArrayState(const ArrayState& rhs) :
    Inherit(rhs)
{
}

ref_ptr<ArrayState> PositionAndDisplacementMapArrayState::cloneArrayState()
{
    return PositionAndDisplacementMapArrayState::create(*this);
}

ref_ptr<ArrayState> PositionAndDisplacementMapArrayState::cloneArrayState(ref_ptr<ArrayState> arrayState)
{
    return PositionAndDisplacementMapArrayState::create(*arrayState);
}

void PositionAndDisplacementMapArrayState::apply(const VertexInputState& vas)
{
    getAttributeDetails(vas, vertex_attribute_location, vertexAttribute);
    getAttributeDetails(vas, normal_attribute_location, normalAttribute);
    getAttributeDetails(vas, texcoord_attribute_location, texcoordAttribute);
    getAttributeDetails(vas, position_attribute_location, positionAttribute);
}

ref_ptr<const vec3Array> PositionAndDisplacementMapArrayState::vertexArray(uint32_t instanceIndex)
{
    auto positions = arrays[positionAttribute.binding].cast<vec3Array>();

    vec3 position;
    if (positions && (instanceIndex < positions->size()))
    {
        position = positions->at(instanceIndex);
    }

    if (displacementMap)
    {
        auto normals = arrays[normalAttribute.binding].cast<vec3Array>();
        auto texcoords = arrays[texcoordAttribute.binding].cast<vec2Array>();
        if (texcoords->size() != vertices->size()) return {};
        if (normals->size() != vertices->size()) return {};

        auto new_vertices = vsg::vec3Array::create(static_cast<uint32_t>(vertices->size()));
        auto src_vertex_itr = vertices->begin();
        auto src_teccoord_itr = texcoords->begin();
        auto src_normal_itr = normals->begin();
        //vec2 tc_scale(static_cast<float>(displacementMap->width()) - 1.0f, static_cast<float>(displacementMap->height()) - 1.0f);

        // if no sampler is assigned fallback to use default constructed Sampler
        if (!sampler) sampler = Sampler::create();

        for (auto& v : *new_vertices)
        {
            const auto& tc = *(src_teccoord_itr++);
            const auto& n = *(src_normal_itr++);
            float d = sample(*sampler, *displacementMap, tc);
            v = *(src_vertex_itr++) + n * d + position;
        }
        return new_vertices;
    }

    return vertices;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BillboardArrayState
//
BillboardArrayState::BillboardArrayState()
{
}

BillboardArrayState::BillboardArrayState(const BillboardArrayState& rhs) :
    Inherit(rhs),
    position_attribute_location(rhs.position_attribute_location),
    positionAttribute(rhs.positionAttribute)
{
}

BillboardArrayState::BillboardArrayState(const ArrayState& rhs) :
    Inherit(rhs)
{
}

ref_ptr<ArrayState> BillboardArrayState::cloneArrayState()
{
    return BillboardArrayState::create(*this);
}

ref_ptr<ArrayState> BillboardArrayState::cloneArrayState(ref_ptr<ArrayState> arrayState)
{
    return BillboardArrayState::create(*arrayState);
}

void BillboardArrayState::apply(const VertexInputState& vas)
{
    getAttributeDetails(vas, vertex_attribute_location, vertexAttribute);
    getAttributeDetails(vas, position_attribute_location, positionAttribute);
}

ref_ptr<const vec3Array> BillboardArrayState::vertexArray(uint32_t instanceIndex)
{
    struct GetValue : public ConstVisitor
    {
        explicit GetValue(uint32_t i) :
            index(i) {}
        uint32_t index;
        vec4 value;

        void apply(const vec4Value& data) override { value = data.value(); }
        void apply(const vec4Array& data) override { value = data[index]; }
    } gv(instanceIndex);

    // get the position_distanceScale value
    arrays[positionAttribute.binding]->accept(gv);
    dvec3 position(gv.value.xyz);
    double autoDistanceScale = gv.value.w;

    dmat4 billboard_to_local;
    if (!localToWorldStack.empty() && !worldToLocalStack.empty())
    {
        const auto& mv = localToWorldStack.back();
        const auto& inverse_mv = worldToLocalStack.back();
        auto center_eye = mv * position;
        auto billboard_mv = computeBillboardMatrix(center_eye, autoDistanceScale);
        billboard_to_local = inverse_mv * billboard_mv;
    }
    else
    {
        billboard_to_local = vsg::translate(position);
    }

    auto new_vertices = vsg::vec3Array::create(static_cast<uint32_t>(vertices->size()));
    auto src_vertex_itr = vertices->begin();
    for (auto& v : *new_vertices)
    {
        const auto& sv = *(src_vertex_itr++);
        v = vec3(billboard_to_local * dvec3(sv));
    }
    return new_vertices;
}
