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

namespace vsg
{

    class VSG_DECLSPEC ArrayState : public Inherit<ConstVisitor, ArrayState>
    {
    public:
        struct AttributeDetails
        {
            uint32_t binding = 0;
            uint32_t offset = 0;
            uint32_t stride = 0;
            VkFormat format = {};
        };

        VkPrimitiveTopology topology;
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

        void apply(uint32_t firstBinding, const DataList& in_arrays);

        void apply(const vsg::vec3Array& array) override;
        void apply(const vsg::Data& array) override;
    };

} // namespace vsg
