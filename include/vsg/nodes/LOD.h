#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Node.h>

#include <algorithm>
#include <array>

namespace vsg
{

    /** Level of Detail Node,
     *  Children should be ordered with the highest resolution Child first, thought to lowest resolution LOD child last.
     *  The Child struct stores the visibleHeightRatio and child that it's associated with.
     *  During culling tHe visibleHeightRatio is used as a ratio of screen height that Bound sphere occupies on screen needs to be at least in order for the associated child to be traversed.
     *  Once on child passes this test no more children are checked, so that no more than on child will ever being traversed in a record traversal.
     *  If no Child pass the visible height test then none of the LOD's children will be visible.
     *  During the record traversals the Bound sphere is also checked against the view frustum so that LOD's also enable view frustum culling for subgraphs so there is no need for a separate CullNode/CullGroup to decorate it. */
    class VSG_DECLSPEC LOD : public Inherit<Node, LOD>
    {
    public:
        LOD();

        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            for (auto& child : node.children) child.node->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        struct Child
        {
            double minimumScreenHeightRatio = 0.0; // 0.0 is always visible
            ref_ptr<Node> node;
        };

        using Children = std::vector<Child, allocator_affinity_nodes<Child>>;

        dsphere bound;
        Children children;

        void addChild(const Child& lodChild) { children.push_back(lodChild); }

    protected:
        virtual ~LOD();
    };
    VSG_type_name(vsg::LOD);

} // namespace vsg
