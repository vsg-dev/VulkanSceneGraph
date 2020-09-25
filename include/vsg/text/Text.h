#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Node.h>
#include <vsg/state/StateGroup.h>
#include <vsg/text/Font.h>
#include <vsg/text/TextLayout.h>

namespace vsg
{

    class VSG_DECLSPEC Text : public Inherit<Node, Text>
    {
    public:

        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            if (node._stategroup) node._stategroup->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        /// settings
        ref_ptr<Font> font;
        ref_ptr<TextLayout> layout;
        std::string text;

        /// create the rendering backend
        virtual void setup();

        /// rendering state used to set up grahics pipeline and descriptor sets, assigned to Font to allow it be be shared
        struct RenderingState : public Inherit<Object, RenderingState>
        {
            RenderingState(Font* font);

            ref_ptr<BindGraphicsPipeline> bindGraphicsPipeline;
            ref_ptr<BindDescriptorSet> bindDescriptorSet;
        };

    protected:

        ref_ptr<RenderingState> _sharedRenderingState;
        ref_ptr<StateGroup> _stategroup;
    };
    VSG_type_name(vsg::Text);

}
