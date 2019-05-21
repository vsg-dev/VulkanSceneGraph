#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/sphere.h>
#include <vsg/nodes/Node.h>

namespace vsg
{

    /** CullNode that enables view frustum culling on a single child node.
     * A valid node must always be assigned to a CullNode before it's used, as, for performance reasons, there are no internal checks made when accessing the child.*/
    class VSG_DECLSPEC CullNode : public Inherit<Node, CullNode>
    {
    public:
        using value_type = TRANSFORM_VALUE_TYPE;
        using Sphere = t_sphere<value_type>;

        CullNode(Allocator* allocator = nullptr);

        CullNode(const Sphere& bound, Node* child, Allocator* allocator = nullptr);

        void traverse(Visitor& visitor) override { _child->accept(visitor); }
        void traverse(ConstVisitor& visitor) const override { _child->accept(visitor); }
        void traverse(DispatchTraversal& visitor) const override { _child->accept(visitor); }
        void traverse(CullTraversal& visitor) const override { _child->accept(visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        void setBound(const Sphere& bound) { _bound = bound; }
        inline const Sphere& getBound() const { return _bound; }

        void setChild(Node* child) { _child = child; }
        Node* getChild() { return _child; }
        const Node* getChild() const { return _child; }

    protected:
        virtual ~CullNode();

        Sphere _bound;
        ref_ptr<vsg::Node> _child;
    };
    VSG_type_name(vsg::CullNode);

} // namespace vsg
