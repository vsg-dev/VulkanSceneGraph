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

    /// CullNode that enables view frustum culling on a single child node.
    /// A valid node must always be assigned to a CullNode before it's used,
    /// for performance reasons there are no internal checks made when accessing the child.*/
    class VSG_DECLSPEC CullNode : public Inherit<Node, CullNode>
    {
    public:
        CullNode();
        CullNode(const dsphere& in_bound, Node* in_child);

        void traverse(Visitor& visitor) override { child->accept(visitor); }
        void traverse(ConstVisitor& visitor) const override { child->accept(visitor); }
        void traverse(RecordTraversal& visitor) const override { child->accept(visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

        dsphere bound;
        ref_ptr<vsg::Node> child;

    protected:
        virtual ~CullNode();
    };
    VSG_type_name(vsg::CullNode);

} // namespace vsg
