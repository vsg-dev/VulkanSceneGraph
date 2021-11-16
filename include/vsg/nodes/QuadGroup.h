#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ref_ptr.h>

#include <vsg/nodes/Node.h>

#include <array>
#include <vector>

namespace vsg
{
    VSG_type_name(vsg::QuadGroup);

    class VSG_DECLSPEC QuadGroup : public Inherit<Node, QuadGroup>
    {
    public:
        QuadGroup(Allocator* allocator = nullptr);

        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            for (int i = 0; i < 4; ++i) node.children[i]->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        using Children = std::array<ref_ptr<vsg::Node>, 4>;
        Children children;

    protected:
        virtual ~QuadGroup();
    };

} // namespace vsg
