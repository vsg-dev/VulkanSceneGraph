#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Array.h>
#include <vsg/maths/sphere.h>
#include <vsg/nodes/Node.h>

namespace vsg
{

    /// InstanceNode provides a mechanism for specifying the translations, rotations and scales (transform arrays) of subgraph
    /// that contains InstanceDraw leaf node(s) that utlize the InstanceNode's per instance transform arrays combined with the
    /// InstanceDraw nodes per vertex arrays.
    class VSG_DECLSPEC InstanceNode : public Inherit<Node, InstanceNode>
    {
    public:
        InstanceNode();
        InstanceNode(const InstanceNode& rhs, const CopyOp& copyop = {});
        InstanceNode(const dsphere& in_bound, Node* in_child);

        vsg::ref_ptr<vec3Array> translations;
        vsg::ref_ptr<quatArray> rotations;
        vsg::ref_ptr<vec3Array> scales;

        dsphere bound;
        ref_ptr<vsg::Node> child;

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return InstanceNode::create(*this, copyop); }
        int compare(const Object& rhs) const override;

        void traverse(Visitor& visitor) override { child->accept(visitor); }
        void traverse(ConstVisitor& visitor) const override { child->accept(visitor); }
        void traverse(RecordTraversal& visitor) const override { child->accept(visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~InstanceNode();
    };
    VSG_type_name(vsg::InstanceNode);

} // namespace vsg
