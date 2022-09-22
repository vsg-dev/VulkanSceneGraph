#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/text/Text.h>

namespace vsg
{
    class TextGroup : public vsg::Inherit<vsg::Node, TextGroup>
    {
    public:

        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            for (auto& child : node.children) child->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal& visitor) const override
        {
            if (!children.empty()) children.front()->accept(visitor);
        }

        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        using Children = std::vector<ref_ptr<Text>, allocator_affinity_nodes<ref_ptr<Text>>>;
        Children children;

        void addChild(ref_ptr<Text> text);

        /// create the rendering backend.
        /// minimumAllocation provides a hint for the minimum number of glyphs to allocate space for.
        virtual void setup(uint32_t minimumAllocation = 0);
    };
    VSG_type_name(vsg::TextGroup);
}
