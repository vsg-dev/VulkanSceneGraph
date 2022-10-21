#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Draw.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/text/TextTechnique.h>

namespace vsg
{
    struct LayoutStruct
    {
        vec3 position = vec3(0.0f, 0.0f, 0.0f);
        float billboardAutoScaleDistance = 0.0;
        vec3 horizontal = vec3(1.0f, 0.0f, 0.0f);
        float horizontalAlignment = 0.0;
        vec3 vertical = vec3(0.0f, 1.0f, 0.0f);
        float verticalAlignment = 0.0;
        vec4 color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        vec4 outlineColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        float outlineWidth = 0.0f;
    };
    VSG_value(TextLayoutValue, LayoutStruct);

    class VSG_DECLSPEC GpuLayoutTechnique : public Inherit<TextTechnique, GpuLayoutTechnique>
    {
    public:
        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            if (node.scenegraph) node.scenegraph->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal& visitor) const override { t_traverse(*this, visitor); }

        void setup(Text* text, uint32_t minimumAllocation = 0, ref_ptr<const Options> options = {}) override;
        void setup(TextGroup* textGroup, uint32_t minimumAllocation = 0, ref_ptr<const Options> options = {}) override;
        dbox extents() const override { return textExtents; }

        // implementation data structure
        dbox textExtents;
        ref_ptr<Node> scenegraph;

        ref_ptr<vec3Array> vertices;
        ref_ptr<Draw> draw;

        ref_ptr<uintArray> textArray;
        ref_ptr<TextLayoutValue> layoutValue;
        ref_ptr<DescriptorBuffer> textDescriptor;
        ref_ptr<DescriptorBuffer> layoutDescriptor;
        ref_ptr<BindDescriptorSet> bindTextDescriptorSet;
        ref_ptr<BindVertexBuffers> bindVertexBuffers;
    };
    VSG_type_name(vsg::GpuLayoutTechnique);

} // namespace vsg
