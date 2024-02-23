#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Allocator.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

namespace vsg
{

    /// Base class for animation samplers that sample animation data and set associated scene graph objects
    class VSG_DECLSPEC AnimationSampler : public Inherit<Visitor, AnimationSampler>
    {
    public:
        AnimationSampler();
        AnimationSampler(const AnimationSampler& rhs, const CopyOp& copyop = {});

        std::string name;

        virtual void update(double time) = 0;
        virtual double maxTime() const = 0;

    public:
        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::AnimationSampler);

    /// Animation class that controls a single animation made up of one more samplers.
    class VSG_DECLSPEC Animation : public Inherit<Object, Animation>
    {
    public:
        Animation();
        Animation(const Animation& rhs, const CopyOp& copyop = {});

        std::string name;

        enum Mode
        {
            ONCE,
            REPEAT,
            FORWARD_AND_BACK
        };

        /// specify how time value outside the sampler maxTime period is managed
        Mode mode = REPEAT;

        /// local animation time used to look up samplers, automatically updated by the Animation::update(..) method.
        double time = 0.0;

        /// speed multiplier for how quickly the Animation::time changes with changes in simulationTime
        double speed = 1.0;

        /// Animation samplers provide the means for mapping the local animation time
        using Samplers = std::vector<ref_ptr<AnimationSampler>>;
        Samplers samplers;

        /// initialize this animation start time
        virtual bool start(double simulationTime, double startTime = 0.0);

        /// update the samplers
        virtual bool update(double simulationTime);

        /// signal that this animation is to stop
        virtual bool stop(double simulationTime);

        /// return true if this animation is being actively updated.
        bool active() const { return _active; }

        /// compute the max time from the highest time keyframes in the available samplers
        virtual double maxTime() const;

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return Animation::create(*this, copyop); }
        int compare(const Object& rhs) const override;

        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            for (auto& sampler : node.samplers) sampler->accept(visitor);
        }
        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        // start time point of animation to be used to calaculate the current time to use when looking up current values
        bool _active = false;
        double _previousSimulationTime = 0.0;
        double _maxTime = 0.0;
    };
    VSG_type_name(vsg::Animation);

    using Animations = std::vector<ref_ptr<Animation>, allocator_affinity_nodes<ref_ptr<Animation>>>;

} // namespace vsg
