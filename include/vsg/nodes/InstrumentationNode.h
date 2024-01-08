#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/sphere.h>
#include <vsg/nodes/Node.h>
#include <vsg/utils/Instrumentation.h>

namespace vsg
{

    /// InstrumentationNode enables instrumentation of a subgraph
    class VSG_DECLSPEC InstrumentationNode : public Inherit<Node, InstrumentationNode>
    {
    public:
        InstrumentationNode();
        InstrumentationNode(ref_ptr<Node> in_child);

        void traverse(Visitor& visitor) override;
        void traverse(ConstVisitor& visitor) const override;
        void traverse(RecordTraversal& visitor) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        void setColor(uint_color color);
        uint_color getColor() const { return _color; }

        void setName(const std::string& name);
        const std::string& getName() const { return _name; }

        void setLevel(uint32_t level);
        uint32_t getLevel() const { return _level; }

        ref_ptr<vsg::Node> child;

    protected:
        virtual ~InstrumentationNode();

        uint32_t _level = 1;
        uint_color _color;
        std::string _name;

        // local caches of setting for passing to Instrumentation
        SourceLocation _sl_Visitor;
        SourceLocation _sl_ConstVisitor;
        SourceLocation _sl_RecordTraversal;
    };
    VSG_type_name(vsg::InstrumentationNode);

} // namespace vsg
