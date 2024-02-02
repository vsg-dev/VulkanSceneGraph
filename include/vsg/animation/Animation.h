#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/Transform.h>
#include <vsg/ui/UIEvent.h>

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

    class VSG_DECLSPEC TransformKeyframes : public Inherit<Object, TransformKeyframes>
    {
    public:
        TransformKeyframes();

        /// name of node
        std::string name;

        /// matrix to update
        ref_ptr<dmat4Value> matrix;

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

        virtual void update(double simulationTime);
    };
    VSG_type_name(vsg::TransformKeyframes);

    class VSG_DECLSPEC MorphKeyframes : public Inherit<Object, MorphKeyframes>
    {
    public:
        MorphKeyframes();

        /// name of animation
        std::string name;

        /// key frames
        std::vector<MorphKey> keyframes;

        void read(Input& input) override;
        void write(Output& output) const override;

        virtual void update(double time);
    };
    VSG_type_name(vsg::MorphKeyframes);

    class VSG_DECLSPEC Animation : public Inherit<Object, Animation>
    {
    public:

        Animation();

        std::string name;

        enum Mode
        {
            ONCE,
            REPEAT,
            FORWARD_AND_BACK
        };

        Mode mode = ONCE;
        double speed = 1.0;

        // start time point of animation to be used to calaculate the current time to use when looking up current values
        double startTime;

        using TransformKeyframesList = std::vector<ref_ptr<TransformKeyframes>>;
        TransformKeyframesList transformKeyframes;

        using MorphKeyframesList = std::vector<ref_ptr<MorphKeyframes>>;
        MorphKeyframesList morphKeyframes;

        void read(Input& input) override;
        void write(Output& output) const override;

        // update
        virtual void update(double simulationTime);
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

        // update
        virtual void update(double simulationTime);

        Animations animations;

        Animations active;

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
