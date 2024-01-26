#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>
#include <vsg/nodes/Group.h>

#include <vector>
namespace vsg
{

    struct VectorKey
    {
        double time;
        dvec3 value;

        bool operator < (const VectorKey& rhs) const { return time < rhs.time; }
    };

    struct QuatKey
    {
        double time;
        dquat value;

        bool operator < (const QuatKey& rhs) const { return time < rhs.time; }
    };

    struct MorphKey
    {
        double time;
        std::vector<unsigned int> values;
        std::vector<double> weights;

        bool operator < (const MorphKey& rhs) const { return time < rhs.time; }
    };

    class VSG_DECLSPEC TransformAnimation : public Inherit<Object, TransformAnimation>
    {
    public:
        TransformAnimation();

        /// name of animation
        std::string name;

        /// position key frames
        std::vector<VectorKey> positions;

        /// rotation key frames
        std::vector<QuatKey> rotations;

        /// scale key frames
        std::vector<VectorKey> scales;

        // assimp pre state ?
        // assimp post state ?

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::TransformAnimation);

    class VSG_DECLSPEC MorphAnimation : public Inherit<Object, MorphAnimation>
    {
    public:
        MorphAnimation();

        /// name of animation
        std::string name;

        /// key frames
        std::vector<MorphKey> keyFrames;

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::MorphAnimation);

    class VSG_DECLSPEC Animation : public Inherit<Object, Animation>
    {
    public:

        Animation();

        std::string name;

        using TransformAnimations = std::vector<ref_ptr<TransformAnimation>>;
        TransformAnimations transformAnimations;

        using MorphAnimations = std::vector<ref_ptr<MorphAnimation>>;
        MorphAnimations morphAnimations;

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::Animation);

    using Animations = std::vector<ref_ptr<Animation>, allocator_affinity_nodes<ref_ptr<Animation>>>;

    /// AnimationGroup node provides a list of children.
    class VSG_DECLSPEC AnimationGroup : public Inherit<Group, AnimationGroup>
    {
    public:
        explicit AnimationGroup(size_t numChildren = 0);

        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        Animations animations;

    protected:
        virtual ~AnimationGroup();
    };
    VSG_type_name(vsg::AnimationGroup);

} // namespace vsg
