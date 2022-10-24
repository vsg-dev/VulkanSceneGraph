#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/text/Font.h>
#include <vsg/text/TextLayout.h>
#include <vsg/text/TextTechnique.h>

namespace vsg
{

    class VSG_DECLSPEC CpuLayoutTechnique : public Inherit<TextTechnique, CpuLayoutTechnique>
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

        virtual ref_ptr<Node> createRenderingSubgraph(ref_ptr<ShaderSet> shaderSet, ref_ptr<Font> font, bool billboard, TextQuads& textQuads, uint32_t minimumAllocation);

        // implementation data structure
        dbox textExtents;
        ref_ptr<Node> scenegraph;

        ref_ptr<vec3Array> vertices;
        ref_ptr<vec4Array> colors;
        ref_ptr<vec4Array> outlineColors;
        ref_ptr<floatArray> outlineWidths;
        ref_ptr<vec3Array> texcoords;
        ref_ptr<vec4Array> centerAndAutoScaleDistances;
        ref_ptr<Data> indices;
        ref_ptr<DrawIndexed> drawIndexed;

        ref_ptr<BindVertexBuffers> bindVertexBuffers;
        ref_ptr<BindIndexBuffer> bindIndexBuffer;
    };
    VSG_type_name(vsg::CpuLayoutTechnique);

} // namespace vsg
