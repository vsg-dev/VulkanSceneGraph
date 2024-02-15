#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/Animation.h>
#include <vsg/animation/Joint.h>

namespace vsg
{

    class VSG_DECLSPEC JointSampler : public Inherit<AnimationSampler, JointSampler>
    {
    public:
        JointSampler();
        JointSampler(const JointSampler& rhs, const CopyOp& copyop = {});

        ref_ptr<mat4Array> jointMatrices;
        std::vector<dmat4> offsetMatrices;
        ref_ptr<Node> subgraph;

        void update(double time) override;
        double maxTime() const override;

        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return JointSampler::create(*this, copyop); }

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        void apply(Node& node) override;
        void apply(Transform& transform) override;
        void apply(MatrixTransform& mt) override;
        void apply(Joint& joint) override;

        std::vector<dmat4> _matrixStack;
    };
    VSG_type_name(vsg::JointSampler);

} // namespace vsg
