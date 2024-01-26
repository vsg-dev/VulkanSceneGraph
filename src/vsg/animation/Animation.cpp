/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/compare.h>
#include <vsg/io/Input.h>
#include <vsg/io/Options.h>
#include <vsg/io/Output.h>
#include <vsg/animation/Animation.h>

using namespace vsg;


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TransformAnimation
//
TransformAnimation::TransformAnimation()
{
}

void TransformAnimation::read(Input& input)
{
    Object::read(input);

    input.read("name", name);

    // read position key frames
    uint32_t num_positions = input.readValue<uint32_t>("positions");
    positions.resize(num_positions);
    for(auto& position : positions)
    {
        input.matchPropertyName("position");
        input.read(1, &position.time);
        input.read(1, &position.value);
    }

    // read rotation key frames
    uint32_t num_rotations = input.readValue<uint32_t>("rotations");
    rotations.resize(num_rotations);
    for(auto& rotation : rotations)
    {
        input.matchPropertyName("rotation");
        input.read(1, &rotation.time);
        input.read(1, &rotation.value);
    }

    // read scale key frames
    uint32_t num_scales = input.readValue<uint32_t>("scales");
    scales.resize(num_scales);
    for(auto& scale : scales)
    {
        input.matchPropertyName("scale");
        input.read(1, &scale.time);
        input.read(1, &scale.value);
    }
}

void TransformAnimation::write(Output& output) const
{
    Object::write(output);

    output.write("name", name);

    // write position key frames
    output.writeValue<uint32_t>("positions", positions.size());
    for(auto& position : positions)
    {
        output.writePropertyName("position");
        output.write(1, &position.time);
        output.write(1, &position.value);
        output.writeEndOfLine();
    }

    // write rotation key frames
    output.writeValue<uint32_t>("rotations", rotations.size());
    for(auto& rotation : rotations)
    {
        output.writePropertyName("rotation");
        output.write(1, &rotation.time);
        output.write(1, &rotation.value);
        output.writeEndOfLine();
    }

    // write scale key frames
    output.writeValue<uint32_t>("scales", scales.size());
    for(auto& scale : scales)
    {
        output.writePropertyName("scale");
        output.write(1, &scale.time);
        output.write(1, &scale.value);
        output.writeEndOfLine();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MorphAnimation
//
MorphAnimation::MorphAnimation()
{
}

void MorphAnimation::read(Input& input)
{
    Object::read(input);

    input.read("name", name);

    // read key frames
    uint32_t num_keyFrames = input.readValue<uint32_t>("keyFrames");
    keyFrames.resize(num_keyFrames);
    for(auto& keyFrame : keyFrames)
    {
        input.read("keyFrame", keyFrame.time);
        input.readValues("values", keyFrame.values);
        input.readValues("weights", keyFrame.weights);
    }
}

void MorphAnimation::write(Output& output) const
{
    Object::write(output);

    output.write("name", name);

    // write key frames
    output.writeValue<uint32_t>("keyFrames", keyFrames.size());
    for(auto& keyFrame : keyFrames)
    {
        output.write("keyFrame", keyFrame.time);
        output.writeValues("values", keyFrame.values);
        output.writeValues("weights", keyFrame.weights);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Animation
//
Animation::Animation()
{
}

void Animation::read(Input& input)
{
    Object::read(input);

    input.read("name", name);

    input.readObjects("transformAnimations", transformAnimations);
    input.readObjects("morphAnimations", morphAnimations);
}

void Animation::write(Output& output) const
{
    Object::write(output);

    output.write("name", name);

    output.writeObjects("transformAnimations", transformAnimations);
    output.writeObjects("morphAnimations", morphAnimations);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// AnimationGroup
//
AnimationGroup::AnimationGroup(size_t numChildren) :
    Inherit(numChildren)
{
}

AnimationGroup::~AnimationGroup()
{
}

int AnimationGroup::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_pointer_container(animations, rhs.animations)) != 0) return result;
    return compare_pointer_container(children, rhs.children);
}

void AnimationGroup::read(Input& input)
{
    Node::read(input);

    input.readObjects("animations", animations);
    input.readObjects("children", children);
}

void AnimationGroup::write(Output& output) const
{
    Node::write(output);

    output.writeObjects("animations", animations);
    output.writeObjects("children", children);
}
