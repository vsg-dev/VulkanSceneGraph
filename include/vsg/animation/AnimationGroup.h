#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/Animation.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/Transform.h>

namespace vsg
{

    /// AnimationGroup node provides a list of child nodes and a list of animations to animate them.
    class VSG_DECLSPEC AnimationGroup : public Inherit<Group, AnimationGroup>
    {
    public:
        explicit AnimationGroup(size_t numChildren = 0);

        Animations animations;

        ref_ptr<Object> clone(CopyOp&) const override;
        int compare(const Object& rhs) const override;

        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            for (auto& animation : node.animations) animation->accept(visitor);
            for (auto& child : node.children) child->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
        void traverse(RecordTraversal& visitor) const override
        {
            for (auto& child : children) child->accept(visitor);
        }

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~AnimationGroup();
    };
    VSG_type_name(vsg::AnimationGroup);

    using AnimationGroups = std::vector<ref_ptr<AnimationGroup>>;

} // namespace vsg
