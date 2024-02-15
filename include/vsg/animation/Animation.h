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

    class VSG_DECLSPEC AnimationSampler : public Inherit<Visitor, AnimationSampler>
    {
    public:
        AnimationSampler();

        std::string name;

        virtual void update(double time) = 0;
        virtual double maxTime() const = 0;

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::AnimationSampler);

    class VSG_DECLSPEC Animation : public Inherit<Object, Animation>
    {
    public:
        Animation();
        Animation(const Animation& rhs, const CopyOp& copyop = {});

        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return Animation::create(*this, copyop); }

        template<class N, class V>
        static void t_traverse(N& node, V& visitor)
        {
            for (auto& sampler : node.samplers) sampler->accept(visitor);
        }

        void traverse(Visitor& visitor) override { t_traverse(*this, visitor); }
        void traverse(ConstVisitor& visitor) const override { t_traverse(*this, visitor); }

        std::string name;

        enum Mode
        {
            ONCE,
            REPEAT,
            FORWARD_AND_BACK
        };

        Mode mode = REPEAT;
        double speed = 1.0;

        // start time point of animation to be used to calaculate the current time to use when looking up current values
        double startTime;

        using Samplers = std::vector<ref_ptr<AnimationSampler>>;
        Samplers samplers;

        void read(Input& input) override;
        void write(Output& output) const override;

        // update
        virtual bool update(double simulationTime);
    };
    VSG_type_name(vsg::Animation);

    using Animations = std::vector<ref_ptr<Animation>, allocator_affinity_nodes<ref_ptr<Animation>>>;

} // namespace vsg
