#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ref_ptr.h>

#include <vsg/nodes/Node.h>

#include <algorithm>
#include <array>

namespace vsg
{
    VSG_type_name(vsg::LOD);

    class VSG_DECLSPEC LOD : public Inherit<Node, LOD>
    {
    public:
        LOD() {}

        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            for (auto& child : node._children) child->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(DispatchTraversal& visitor) const override { t_traverse(*this, visitor); }
        void traverse(CullTraversal& visitor) const override { t_traverse(*this, visitor); }

        void setBound(const dsphere& bound) { _bound = bound; }
        const dsphere& getBound() const { return _bound; }

        /// set the minimum screen space area that a child is visible from
        void setMinimumArea(std::size_t pos, double area) { _minimumAreas[pos] = area; }
        double getMinimumArea(std::size_t pos) const { return _minimumAreas[pos]; }

        void setChild(std::size_t pos, Node* node) { _children[pos] = node; }
        Node* getChild(std::size_t pos) { return _children[pos].get(); }
        const Node* getChild(std::size_t pos) const { return _children[pos].get(); }

        std::size_t getNumChildren() const { return 2; }

        using MinimumAreas = std::array<double, 2>;
        MinimumAreas& getMinimumAreas() { return _minimumAreas; }
        const MinimumAreas& getMinimumAreas() const { return _minimumAreas; }

        using Children = std::array<ref_ptr<Node>, 2>;
        Children& getChildren() { return _children; }
        const Children& getChildren() const { return _children; }

    protected:
        virtual ~LOD() {}

        dsphere _bound;
        MinimumAreas _minimumAreas;
        Children _children;
    };

} // namespace vsg
