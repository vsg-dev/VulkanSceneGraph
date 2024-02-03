#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/nodes/Group.h>
#include <vsg/nodes/Transform.h>
#include <vsg/animation/Animation.h>

namespace vsg
{

    /// AnimationGroup node provides a list of child nodes and a list of animations to animate them.
    class VSG_DECLSPEC AnimationGroup : public Inherit<Group, AnimationGroup>
    {
    public:
        explicit AnimationGroup(size_t numChildren = 0);

        Animations animations;

        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~AnimationGroup();
    };
    VSG_type_name(vsg::AnimationGroup);

    using AnimationGroups = std::vector<vsg::ref_ptr<vsg::AnimationGroup>>;

    /// AnimationTransform node provides a list of children.
    class VSG_DECLSPEC AnimationTransform : public Inherit<Transform, AnimationTransform>
    {
    public:
        explicit AnimationTransform();

        std::string name;

        ref_ptr<dmat4Value> matrix;

        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        dmat4 transform(const dmat4& mv) const override;

    protected:
        virtual ~AnimationTransform();
    };
    VSG_type_name(vsg::AnimationTransform);

    /// AnimationTransform node provides a list of children.
    class VSG_DECLSPEC RiggedTransform : public Inherit<Node, RiggedTransform>
    {
    public:
        explicit RiggedTransform();

        std::string name;

        ref_ptr<dmat4Value> matrix;

        using Children = std::vector<ref_ptr<Node>, allocator_affinity_nodes<ref_ptr<Node>>>;
        Children children;

        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~RiggedTransform();
    };
    VSG_type_name(vsg::RiggedTransform);

} // namespace vsg
