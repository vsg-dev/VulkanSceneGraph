#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ConstVisitor.h>
#include <vsg/core/Data.h>
#include <vsg/core/Inherit.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/state/Sampler.h>

namespace vsg
{

    /// ArrayState base class provides a mechanism for CPU mapping of array data that is processed in novel ways on the GPU.
    /// Assigned to StategGroup that are associated GraphicsPipline that use vertex shaders with novel vertex processing,
    /// and used during traversal such as ComputeTraversal and IntersectionTraversal.
    class VSG_DECLSPEC ArrayState : public Inherit<ConstVisitor, ArrayState>
    {
    public:
        ArrayState() = default;
        ArrayState(const ArrayState&) = default;

        /// clone self
        virtual ref_ptr<ArrayState> clone()
        {
            return ArrayState::create(*this);
        }

        // clone the specified ArrayState
        virtual ref_ptr<ArrayState> clone(ref_ptr<ArrayState> arrayState)
        {
            return ArrayState::create(*arrayState);
        }

        struct AttributeDetails
        {
            uint32_t binding = 0;
            VkFormat format = {};
            uint32_t stride = 0;
            uint32_t offset = 0;
            VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        };

        std::vector<dmat4> localToWorldStack;
        std::vector<dmat4> worldToLocalStack;

        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        uint32_t vertex_attribute_location = 0;
        AttributeDetails vertexAttribute;

        ref_ptr<const vec3Array> vertices;
        ref_ptr<vec3Array> proxy_vertices;

        DataList arrays;

        bool getAttributeDetails(const VertexInputState& vas, uint32_t location, AttributeDetails& attributeDetails);

        using ConstVisitor::apply;

        void apply(const BindGraphicsPipeline& bpg) override;
        void apply(const VertexInputState& vas) override;
        void apply(const InputAssemblyState& ias) override;

        void apply(const Geometry& geometry) override;
        void apply(const VertexDraw& vid) override;
        void apply(const VertexIndexDraw& vid) override;
        void apply(const BindVertexBuffers& bvb) override;
        void apply(const BufferInfo& bufferInfo) override;

        void apply(const vec3Array& array) override;
        void apply(const Data& array) override;

        virtual void applyArrays(uint32_t firstBinding, const BufferInfoList& in_arrays);
        virtual void applyArrays(uint32_t firstBinding, const DataList& in_arrays);
        virtual ref_ptr<const vec3Array> vertexArray(uint32_t instanceIndex);

    protected:
        virtual ~ArrayState() {}
    };
    VSG_type_name(vsg::ArrayState);

    /// NullArrayState provides a mechanism for geometry in a subgraph to be ignored by traversals that use ArrayState such as ComputeBounds/Intersection/LineSegmentIntersector
    /// this is useful for subgraphs that have custom shaders that move the final rendered geometry to a different place that would be naively interpreted by a straight forward vec3Array vertex array in local coordinates.
    /// To disable the handling of geometry in a subgraph simple assign a NullArrayState to the StateGroup::prototypeArrayState, i.e.
    ///     stateGroup->prototypeArrayState = vsg::NullArrayState::create();
    class VSG_DECLSPEC NullArrayState : public Inherit<ArrayState, NullArrayState>
    {
    public:
        NullArrayState();
        explicit NullArrayState(const ArrayState& as);

        ref_ptr<ArrayState> clone() override;
        ref_ptr<ArrayState> clone(ref_ptr<ArrayState> arrayState) override;

        using ArrayState::apply;

        void apply(const vec3Array&) override;
        void apply(const Data&) override;
    };
    VSG_type_name(vsg::NullArrayState);

    /// PositionArrayState is ArrayState subclass for mapping vertex array data for instanced geometries.
    class VSG_DECLSPEC PositionArrayState : public Inherit<ArrayState, PositionArrayState>
    {
    public:
        PositionArrayState();
        PositionArrayState(const PositionArrayState& rhs);
        explicit PositionArrayState(const ArrayState& rhs);

        ref_ptr<ArrayState> clone() override;
        ref_ptr<ArrayState> clone(ref_ptr<ArrayState> arrayState) override;

        uint32_t position_attribute_location = 4;
        AttributeDetails positionAttribute;

        using ArrayState::apply;

        void apply(const VertexInputState& vas) override;
        ref_ptr<const vec3Array> vertexArray(uint32_t instanceIndex) override;
    };
    VSG_type_name(vsg::PositionArrayState);

    /// DisplacementMapArrayState is ArrayState subclass for mapping vertex array data for displacement mapped geometries.
    class VSG_DECLSPEC DisplacementMapArrayState : public Inherit<ArrayState, DisplacementMapArrayState>
    {
    public:
        DisplacementMapArrayState();
        DisplacementMapArrayState(const DisplacementMapArrayState& rhs);
        explicit DisplacementMapArrayState(const ArrayState& rhs);

        ref_ptr<ArrayState> clone() override;
        ref_ptr<ArrayState> clone(ref_ptr<ArrayState> arrayState) override;

        // binding of displacement map
        uint32_t normal_attribute_location = 1;
        uint32_t texcoord_attribute_location = 2;
        uint32_t dm_set = 0;
        uint32_t dm_binding = 6;

        // displacement map found during traversal
        ref_ptr<floatArray2D> displacementMap;
        ref_ptr<Sampler> sampler;
        AttributeDetails normalAttribute;
        AttributeDetails texcoordAttribute;

        using ArrayState::apply;

        void apply(const DescriptorImage& di) override;
        void apply(const DescriptorSet& ds) override;
        void apply(const BindDescriptorSet& bds) override;
        void apply(const BindDescriptorSets& bds) override;
        void apply(const VertexInputState& vas) override;
        ref_ptr<const vec3Array> vertexArray(uint32_t instanceIndex) override;
    };
    VSG_type_name(vsg::DisplacementMapArrayState);

    /// PositionAndDisplacementMapArrayState is ArrayState subclass for mapping vertex array data for instanced, displacement mapped geometries.
    class VSG_DECLSPEC PositionAndDisplacementMapArrayState : public Inherit<DisplacementMapArrayState, PositionAndDisplacementMapArrayState>
    {
    public:
        PositionAndDisplacementMapArrayState();
        PositionAndDisplacementMapArrayState(const PositionAndDisplacementMapArrayState& rhs);
        explicit PositionAndDisplacementMapArrayState(const ArrayState& rhs);

        uint32_t position_attribute_location = 4;
        AttributeDetails positionAttribute;

        ref_ptr<ArrayState> clone() override;
        ref_ptr<ArrayState> clone(ref_ptr<ArrayState> arrayState) override;

        void apply(const VertexInputState& vas) override;
        ref_ptr<const vec3Array> vertexArray(uint32_t instanceIndex) override;
    };
    VSG_type_name(vsg::PositionAndDisplacementMapArrayState);

    /// BillboardArrayState is ArrayState subclass for mapping vertex array data for billboard instanced geometries.
    class VSG_DECLSPEC BillboardArrayState : public Inherit<ArrayState, BillboardArrayState>
    {
    public:
        BillboardArrayState();
        BillboardArrayState(const BillboardArrayState& rhs);
        explicit BillboardArrayState(const ArrayState& rhs);

        ref_ptr<ArrayState> clone() override;
        ref_ptr<ArrayState> clone(ref_ptr<ArrayState> arrayState) override;

        uint32_t position_attribute_location = 4;
        AttributeDetails positionAttribute;

        using ArrayState::apply;

        void apply(const VertexInputState& vas) override;
        ref_ptr<const vec3Array> vertexArray(uint32_t instanceIndex) override;
    };
    VSG_type_name(vsg::BillboardArrayState);

} // namespace vsg
