#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ref_ptr.h>

#include <vsg/nodes/Node.h>

#include <vector>

namespace vsg
{

    class VSG_DECLSPEC Group : public Inherit<Node, Group>
    {
    public:
        explicit Group(size_t numChildren = 0);
        explicit Group(Allocator* allocator, size_t numChildren = 0);

        template<typename Iterator>
        Group(Iterator begin, Iterator end)
        {
            for (Iterator itr = begin; itr != end; ++itr) addChild(*itr);
        }

        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            for (auto& child : node.children) child->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        using Children = std::vector<ref_ptr<vsg::Node>>;
        Children children;

        void addChild(vsg::ref_ptr<Node> child)
        {
            children.push_back(child);
        }

    protected:
        virtual ~Group();
    };
    VSG_type_name(vsg::Group);

} // namespace vsg
