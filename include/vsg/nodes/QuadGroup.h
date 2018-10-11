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

//#define USE_std_array

namespace vsg
{
    class VSG_EXPORT QuadGroup : public Inherit<Node, QuadGroup>
    {
    public:
        QuadGroup();

        template<class N, class V> static void t_traverse(N& node, V& visitor) { for(int i=0; i<4; ++i) node._children[i]->accept(visitor); }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(DispatchTraversal& visitor) const override { t_traverse(*this, visitor); }
        void traverse(CullTraversal& visitor) const override { t_traverse(*this, visitor); }

        void setChild(std::size_t pos, vsg::Node* node) { _children[pos] = node; }
        vsg::Node* getChild(std::size_t pos) { return _children[pos].get(); }
        const vsg::Node* getChild(std::size_t pos) const { return _children[pos].get(); }

        constexpr std::size_t getNumChildren() const noexcept { return 4; }

#ifdef USE_std_array
        using Children = std::array< ref_ptr< vsg::Node>, 4 >;
#else
        using Children = ref_ptr< vsg::Node>[4];
#endif

        Children& getChildren() noexcept { return _children; }
        const Children& getChildren() const noexcept { return _children; }

    protected:

        virtual ~QuadGroup();

        Children _children;
    };

}
