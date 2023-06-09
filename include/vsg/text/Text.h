#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Node.h>
#include <vsg/text/Font.h>
#include <vsg/text/TextLayout.h>
#include <vsg/text/TextTechnique.h>
#include <vsg/utils/ShaderSet.h>

namespace vsg
{

    /** Text node provides high quality text rendering using signed distance field glyph texture atlas.
      * Text does not provide view frustum culling or level of detail, but you can add this if require
      * it by decorating the Text with a CullNode/LOD and after TextGroup::setup() is called to initialize
      * the rendering component you can use the TextGroup->technique->extents() value to help set the
      * CullNode/LOD.bounds value.*/
    class VSG_DECLSPEC Text : public Inherit<Node, Text>
    {
    public:
        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            if (node.technique) node.technique->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        /// settings
        ref_ptr<Font> font;
        ref_ptr<ShaderSet> shaderSet;
        ref_ptr<TextTechnique> technique;
        ref_ptr<TextLayout> layout;
        ref_ptr<Data> text;

        /// create the rendering backend.
        /// minimumAllocation provides a hint for the minimum number of glyphs to allocate space for.
        virtual void setup(uint32_t minimumAllocation = 0, ref_ptr<const Options> options = {});

    protected:
    };
    VSG_type_name(vsg::Text);

    /// create a ShaderSet used for both CpuALayutTechnique and GpuALayutTechnique or return the Options::shaderSet["text"] entry if available.
    extern VSG_DECLSPEC ref_ptr<ShaderSet> createTextShaderSet(ref_ptr<const Options> options = {});

    /// convenience class for counting the number of text glyphs
    struct VSG_DECLSPEC CountGlyphs : public ConstVisitor
    {
        size_t count = 0;

        void apply(const stringValue& text) override;
        void apply(const wstringValue& text) override;
        void apply(const ubyteArray& text) override;
        void apply(const ushortArray& text) override;
        void apply(const uintArray& text) override;
    };
    VSG_type_name(vsg::CountGlyphs);

} // namespace vsg
