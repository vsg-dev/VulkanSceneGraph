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

namespace vsg
{

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
            uint32_t offset = 0;
            uint32_t stride = 0;
            VkFormat format = {};
        };

        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        uint32_t vertex_attribute_location = 0;
        AttributeDetails vertexAttribute;

        ref_ptr<const vec3Array> vertices;
        ref_ptr<vec3Array> proxy_vertices;

        DataList arrays;

        void apply(const BindGraphicsPipeline& bpg) override;
        void apply(const VertexInputState& vas) override;
        void apply(const InputAssemblyState& ias) override;

        void apply(const Geometry& geometry) override;
        void apply(const VertexIndexDraw& vid) override;
        void apply(const BindVertexBuffers& bvb) override;
        void apply(const BufferInfo& bufferInfo) override;

        void apply(const vec3Array& array) override;
        void apply(const Data& array) override;

        virtual void apply(uint32_t firstBinding, const BufferInfoList& in_arrays);
        virtual void apply(uint32_t firstBinding, const DataList& in_arrays);

    protected:
        virtual ~ArrayState() {}
    };
    VSG_type_name(vsg::ArrayState);

    /// NullArrayState provides a mechanism for geometry in a subgraph to be ignored by traverals that use ArrayState such as ComputeBounds/Intersection/LineSegmentIntersector
    /// this is usefl for subgarphs that have custom shaders that move the final rendered geometry to a different place that would be nievely interpreted by a straight forward vec3Array vertex array in local coordinates.
    /// To disable the handling of geometry in a subgraph simple assign a NullArrayState to the StateGroup::prototypeArrayState, i.e.
    ///     stateGroup->prototypeArrayState = vsg::NullArrayState::create();
    class VSG_DECLSPEC NullArrayState : public Inherit<ArrayState, NullArrayState>
    {
    public:
        NullArrayState();
        NullArrayState(const ArrayState& as);

        ref_ptr<ArrayState> clone() override;
        ref_ptr<ArrayState> clone(ref_ptr<ArrayState> arrayState) override;

        void apply(const vsg::vec3Array&) override;
        void apply(const vsg::Data&) override;
    };
    VSG_type_name(vsg::NullArrayState);

} // namespace vsg
