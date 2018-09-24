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

namespace vsg
{
    class VSG_EXPORT QuadGroup : public vsg::Node
    {
    public:
        QuadGroup() {}

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        inline virtual void traverse(Visitor& visitor)
        {
            for (auto child : _children)
            {
                /*if (child.valid())*/ child->accept(visitor);
            }
        }

        void setChild(std::size_t pos, vsg::Node* node)
        {
            _children[pos] = node;
        }

        vsg::Node* getChild(std::size_t pos) { return _children[pos].get(); }
        const vsg::Node* getChild(std::size_t pos) const { return _children[pos].get(); }

        std::size_t getNumChildren() const { return 4; }

        using Children = std::array< ref_ptr< vsg::Node>, 4 >;

        Children& getChildren() { return _children; }
        const Children& getChildren() const { return _children; }

    protected:

        virtual ~QuadGroup() {}

        Children _children;
    };

}
