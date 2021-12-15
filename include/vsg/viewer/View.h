#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Group.h>

#include <vsg/viewer/Camera.h>
#include <vsg/viewer/Window.h>

namespace vsg
{
    /// View class is Group class that pairs a Camera that defines the view with a subgraph that defines the scene that is being viewed/rendered
    class VSG_DECLSPEC View : public Inherit<Group, View>
    {
    public:
        View();

        explicit View(ref_ptr<Camera> in_camera, ref_ptr<Node> in_scenegraph = {});

        template<class N, class V>
        static void t_accept(N& node, V& visitor)
        {
            if ((visitor.traversalMask & (visitor.overrideMask | node.mask)) == 0) return;

            uint32_t cached_traversalMask = visitor.traversalMask;

            visitor.traversalMask = visitor.traversalMask & node.mask;

            visitor.apply(node);

            visitor.traversalMask = cached_traversalMask;
        }

        void accept(Visitor& visitor) override { t_accept(*this, visitor); }
        void accept(ConstVisitor& visitor) const override { t_accept(*this, visitor); }
        void accept(RecordTraversal& visitor) const override { t_accept(*this, visitor); }

        /// camera controls the viewport state and projection and view matrices
        ref_ptr<Camera> camera;

        /// viewID is automatically assigned in View constructor
        const uint32_t viewID = 0;

        /// mask that controls traversal of the View's subgraph
        /// View is visited if the (visitor.traversalMask & view.mask) != 0,
        /// and when it is visited the visitor.traversalMask is &'ed with the mask to give the traversalMask to use in the subgraph.
        uint32_t mask = 0xffffff;

        /// bins
        std::vector<ref_ptr<Bin>> bins;

    protected:
        virtual ~View();
    };
    VSG_type_name(vsg::View);

} // namespace vsg
