/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/Animation.h>
#include <vsg/core/CopyOp.h>
#include <vsg/core/compare.h>
#include <vsg/io/Input.h>
#include <vsg/io/Options.h>
#include <vsg/io/Output.h>
#include <vsg/nodes/MatrixTransform.h>

using namespace vsg;

AnimationSampler::AnimationSampler()
{
}

void AnimationSampler::read(Input& input)
{
    Object::read(input);
    input.read("name", name);
}

void AnimationSampler::write(Output& output) const
{
    Object::write(output);
    output.write("name", name);
}

Animation::Animation()
{
}

Animation::Animation(const Animation& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    name(rhs.name),
    mode(rhs.mode),
    speed(rhs.speed),
    startTime(rhs.startTime),
    samplers(copyop(rhs.samplers))
{
}

void Animation::read(Input& input)
{
    Object::read(input);

    input.read("name", name);
    input.readObjects("samplers", samplers);
}

void Animation::write(Output& output) const
{
    Object::write(output);

    output.write("name", name);
    output.writeObjects("samplers", samplers);
}

bool Animation::update(double simulationTime)
{
    double maxTime = 0.0;
    for (auto sampler : samplers)
    {
        maxTime = std::max(maxTime, sampler->maxTime());
    }

    // TODO: need to use delta since last time update...
    double time = (simulationTime - startTime) * speed;
    if (mode == REPEAT)
    {
        time = std::fmod(time, maxTime);
    }
    else if (mode == FORWARD_AND_BACK)
    {
        time = std::fmod(time, 2.0 * maxTime);
        if (time > maxTime) time = 2.0 * maxTime - time;
    }
    else
    {
        if (time > maxTime) return false;
    }

    for (auto sampler : samplers)
    {
        sampler->update(time);
    }

    return true;
}
