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
// TransformKeyframes
//
TransformKeyframes::TransformKeyframes()
{
}

void TransformKeyframes::read(Input& input)
{
    Object::read(input);

    input.read("name", name);
    input.read("matrix", matrix);

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

void TransformKeyframes::write(Output& output) const
{
    Object::write(output);

    output.write("name", name);
    output.write("matrix", matrix);

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
    for(auto& keyframe : keyframes)
    {
        input.read("keyFrame", keyframe.time);
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
    for(auto& keyframe : keyframes)
    {
        output.write("keyframe", keyframe.time);
        output.writeValues("values", keyframe.values);
        output.writeValues("weights", keyframe.weights);
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

    input.readObjects("transformKeyframes", transformKeyframes);
    input.readObjects("morphKeyframes", morphKeyframes);
}

void Animation::write(Output& output) const
{
    Object::write(output);

    output.write("name", name);

    output.writeObjects("transformKeyframes", transformKeyframes);
    output.writeObjects("morphKeyframes", morphKeyframes);
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


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// AnimationTransform
//
AnimationTransform::AnimationTransform() :
    Inherit()
{
}

AnimationTransform::~AnimationTransform()
{
}

int AnimationTransform::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(name, rhs.name)) != 0) return result;
    if ((result = compare_pointer(matrix, rhs.matrix)) != 0) return result;
    return compare_pointer_container(children, rhs.children);
}

void AnimationTransform::read(Input& input)
{
    Node::read(input);

    input.read("name", name);
    input.read("matrix", matrix);
    input.readObjects("children", children);

    // TODO : subgraphRequiresLocalFrustum
}

void AnimationTransform::write(Output& output) const
{
    Node::write(output);

    output.write("name", name);
    output.write("matrix", matrix);
    output.writeObjects("children", children);

    // TODO : subgraphRequiresLocalFrustum
}

dmat4 AnimationTransform::transform(const dmat4& mv) const
{
    return mv * matrix->value();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// RiggedTransform
//
RiggedTransform::RiggedTransform() :
    Inherit()
{
}

RiggedTransform::~RiggedTransform()
{
}

int RiggedTransform::compare(const Object& rhs_object) const
{
    int result = Object::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_value(name, rhs.name)) != 0) return result;
    if ((result = compare_pointer(matrix, rhs.matrix)) != 0) return result;
    return compare_pointer_container(children, rhs.children);
}

void RiggedTransform::read(Input& input)
{
    Node::read(input);

    input.read("name", name);
    input.read("matrix", matrix);
    input.readObjects("children", children);

    // TODO : subgraphRequiresLocalFrustum
}

void RiggedTransform::write(Output& output) const
{
    Node::write(output);

    output.write("name", name);
    output.write("matrix", matrix);
    output.writeObjects("children", children);

    // TODO : subgraphRequiresLocalFrustum
}
