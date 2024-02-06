/* <editor-fold desc="MIT License">
`
Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/AnimationGroup.h>
#include <vsg/animation/JointSampler.h>
#include <vsg/core/compare.h>
#include <vsg/io/Input.h>
#include <vsg/io/Options.h>
#include <vsg/io/Output.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/state/DescriptorBuffer.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// JointSampler
//
JointSampler::JointSampler()
{
}

void JointSampler::update(double)
{
    if (!jointMatrices) return;

    _jointIndex = 0;
    _matrixStack.clear();
    _matrixStack.emplace_back();

    if (transformSubgraph) transformSubgraph->accept(*this);

    jointMatrices->dirty();
}

double JointSampler::maxTime() const
{
    double maxTime = 0.0;
    return maxTime;
}

void JointSampler::read(Input& input)
{
    AnimationSampler::read(input);
    input.read("jointMatrices", jointMatrices);
    input.readValues("offsetMatrices", offsetMatrices);
    input.read("transformSubgraph", transformSubgraph);
}

void JointSampler::write(Output& output) const
{
    AnimationSampler::write(output);
    output.write("jointMatrices", jointMatrices);
    output.writeValues("offsetMatrices", offsetMatrices);
    output.write("transformSubgraph", transformSubgraph);
}

void JointSampler::apply(Transform& transform)
{
    auto matrix = transform.transform(_matrixStack.back());
    jointMatrices->set(_jointIndex, mat4(matrix * offsetMatrices[_jointIndex]));

    if (!transform.children.empty())
    {
        _matrixStack.push_back(matrix);
        transform.traverse(*this);
        _matrixStack.pop_back();
    }
}

void JointSampler::apply(MatrixTransform& mt)
{
    auto matrix = _matrixStack.back() * mt.matrix;
    jointMatrices->set(_jointIndex, mat4(matrix * offsetMatrices[_jointIndex]));

    if (!mt.children.empty())
    {
        _matrixStack.push_back(matrix);
        mt.traverse(*this);
        _matrixStack.pop_back();
    }
}

void JointSampler::apply(AnimationTransform& at)
{
    auto matrix = _matrixStack.back() * at.matrix->value();
    jointMatrices->set(_jointIndex, mat4(matrix * offsetMatrices[_jointIndex]));

    if (!at.children.empty())
    {
        _matrixStack.push_back(matrix);
        at.traverse(*this);
        _matrixStack.pop_back();
    }
}

void JointSampler::apply(RiggedTransform& rt)
{
    auto matrix = _matrixStack.back() * rt.matrix->value();
    jointMatrices->set(_jointIndex, mat4(matrix * offsetMatrices[_jointIndex]));

    if (!rt.children.empty())
    {
        _matrixStack.push_back(matrix);
        rt.traverse(*this);
        _matrixStack.pop_back();
    }
}
