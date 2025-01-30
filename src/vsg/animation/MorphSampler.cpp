/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/MorphSampler.h>
#include <vsg/core/compare.h>
#include <vsg/io/Input.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Output.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MorphKeyframes
//
MorphKeyframes::MorphKeyframes()
{
}

void MorphKeyframes::read(Input& input)
{
    Object::read(input);

    input.read("name", name);

    // read key frames
    uint32_t num_keyframes = input.readValue<uint32_t>("keyframes");
    keyframes.resize(num_keyframes);
    for (auto& keyframe : keyframes)
    {
        input.read("time", keyframe.time);
        input.readValues("values", keyframe.values);
        input.readValues("weights", keyframe.weights);
    }
}

void MorphKeyframes::write(Output& output) const
{
    Object::write(output);

    output.write("name", name);

    // write key frames
    output.writeValue<uint32_t>("keyFrames", keyframes.size());
    for (auto& keyframe : keyframes)
    {
        output.write("time", keyframe.time);
        output.writeValues("values", keyframe.values);
        output.writeValues("weights", keyframe.weights);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MorphSampler
//
MorphSampler::MorphSampler()
{
}

MorphSampler::MorphSampler(const MorphSampler& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    keyframes(copyop(rhs.keyframes)),
    object(copyop(rhs.object))
{
}

int MorphSampler::compare(const Object& rhs_object) const
{
    int result = AnimationSampler::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_pointer(keyframes, rhs.keyframes)) != 0) return result;
    return compare_pointer(object, rhs.object);
}

void MorphSampler::update(double /*time*/)
{
    // TODO write implementation of passing morph values to associated scene graph data structures
    vsg::warn("MorphSampler::update(double time) not implemented yet");
}

double MorphSampler::maxTime() const
{
    double maxTime = 0.0;
    if (keyframes && !keyframes->keyframes.empty())
    {
        maxTime = std::max(maxTime, keyframes->keyframes.back().time);
    }
    return maxTime;
}

void MorphSampler::read(Input& input)
{
    AnimationSampler::read(input);
    input.read("keyframes", keyframes);
    input.read("object", object);
}

void MorphSampler::write(Output& output) const
{
    AnimationSampler::write(output);
    output.write("keyframes", keyframes);
    output.write("object", object);
}
