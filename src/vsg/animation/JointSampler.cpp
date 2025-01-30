/* <editor-fold desc="MIT License">
`
Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/AnimationGroup.h>
#include <vsg/animation/Joint.h>
#include <vsg/animation/JointSampler.h>
#include <vsg/core/compare.h>
#include <vsg/io/Input.h>
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

JointSampler::JointSampler(const JointSampler& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    jointMatrices(copyop(rhs.jointMatrices)),
    offsetMatrices(rhs.offsetMatrices),
    subgraph(copyop(rhs.subgraph))
{
}

int JointSampler::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_pointer(jointMatrices, rhs.jointMatrices)) != 0) return result;
    if ((result = compare_value_container(offsetMatrices, rhs.offsetMatrices)) != 0) return result;
    return compare_pointer(subgraph, rhs.subgraph);
}

void JointSampler::update(double)
{
    if (!jointMatrices) return;

    _matrixStack.clear();
    _matrixStack.push_back(dmat4());

    if (subgraph)
    {
        subgraph->accept(*this);
    }

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
    input.read("subgraph", subgraph);
}

void JointSampler::write(Output& output) const
{
    AnimationSampler::write(output);
    output.write("jointMatrices", jointMatrices);
    output.writeValues("offsetMatrices", offsetMatrices);
    output.write("subgraph", subgraph);
}

void JointSampler::apply(Node& node)
{
    node.traverse(*this);
}

void JointSampler::apply(Transform& transform)
{
    if (!transform.children.empty())
    {
        _matrixStack.push_back(transform.transform(_matrixStack.back()));

        transform.traverse(*this);

        _matrixStack.pop_back();
    }
}

void JointSampler::apply(MatrixTransform& mt)
{
    if (!mt.children.empty())
    {
        _matrixStack.push_back(_matrixStack.back() * mt.matrix);

        mt.traverse(*this);

        _matrixStack.pop_back();
    }
}

void JointSampler::apply(Joint& joint)
{
    auto matrix = _matrixStack.back() * joint.matrix;
    jointMatrices->set(joint.index, mat4(matrix * offsetMatrices[joint.index]));

    if (!joint.children.empty())
    {
        _matrixStack.push_back(matrix);

        for (auto& child : joint.children)
        {
            // apply(*child);
            child->accept(*this);
        }

        _matrixStack.pop_back();
    }
}
