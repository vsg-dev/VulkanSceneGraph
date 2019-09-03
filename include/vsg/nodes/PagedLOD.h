#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ref_ptr.h>

#include <vsg/nodes/Node.h>

#include <vsg/io/FileSystem.h>

#include <algorithm>
#include <array>

namespace vsg
{

    /** Level of Detail Node,
     *  Children should be ordered with the highest resolution PagedLODChild first, thought to lowest resolution PagedLOD child last.
     *  The PagedLODChild struct stores the visibleHeightRatio and child that it's associated with.
     *  During culling tHe visibleHeightRatio is used as a ratio of screen height that Bound sphere occupies on screen needs to be at least in order for the associated child to be traversed.
     *  Once on child passes this test no more children are checked, so that no more than on child will ever being traversed in a cull or dispatch traversal.
     *  If no PagedLODChild pass the visible height test then none of the PagedLOD's children will be visible.
     *  During the cull or dispatch traversals the Bound sphere is also checked against the view frustum so that PagedLOD's also enable view frustum culling for subgraphs so there is no need for a separate CullNode/CullGroup to decorate it. */
    class VSG_DECLSPEC PagedLOD : public Inherit<Node, PagedLOD>
    {
    public:
        PagedLOD(Allocator* allocator = nullptr);

        struct PagedLODChild
        {
            double minimumScreenHeightRatio = 0.0; // 0.0 is always visible
            Path filename;
            ref_ptr<Node> child;
        };

        using Children = std::vector<PagedLODChild>;

        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            for (auto& lodChild : node._children) { if (lodChild.child) lodChild.child->accept(visitor); }
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(DispatchTraversal& visitor) const override { t_traverse(*this, visitor); }
        void traverse(CullTraversal& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        void setBound(const dsphere& bound) { _bound = bound; }
        inline const dsphere& getBound() const { return _bound; }

        void setChild(std::size_t pos, const PagedLODChild& lodChild) { _children[pos] = lodChild; }
        PagedLODChild& getChild(std::size_t pos) { return _children[pos]; }
        const PagedLODChild& getChild(std::size_t pos) const { return _children[pos]; }

        void addChild(const PagedLODChild& lodChild) { _children.emplace_back(lodChild); }

        std::size_t getNumChildren() const { return _children.size(); }

        Children& getChildren() { return _children; }
        const Children& getChildren() const { return _children; }

    protected:
        virtual ~PagedLOD();

        dsphere _bound;
        Children _children;
    };
    VSG_type_name(vsg::PagedLOD);

} // namespace vsg
