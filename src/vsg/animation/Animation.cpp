/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/Animation.h>
#include <vsg/core/compare.h>
#include <vsg/io/Input.h>
#include <vsg/io/Options.h>
#include <vsg/io/Output.h>
#include <vsg/nodes/MatrixTransform.h>

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
    for (auto& position : positions)
    {
        input.matchPropertyName("position");
        input.read(1, &position.time);
        input.read(1, &position.value);
    }

    // read rotation key frames
    uint32_t num_rotations = input.readValue<uint32_t>("rotations");
    rotations.resize(num_rotations);
    for (auto& rotation : rotations)
    {
        input.matchPropertyName("rotation");
        input.read(1, &rotation.time);
        input.read(1, &rotation.value);
    }

    // read scale key frames
    uint32_t num_scales = input.readValue<uint32_t>("scales");
    scales.resize(num_scales);
    for (auto& scale : scales)
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
    for (auto& position : positions)
    {
        output.writePropertyName("position");
        output.write(1, &position.time);
        output.write(1, &position.value);
        output.writeEndOfLine();
    }

    // write rotation key frames
    output.writeValue<uint32_t>("rotations", rotations.size());
    for (auto& rotation : rotations)
    {
        output.writePropertyName("rotation");
        output.write(1, &rotation.time);
        output.write(1, &rotation.value);
        output.writeEndOfLine();
    }

    // write scale key frames
    output.writeValue<uint32_t>("scales", scales.size());
    for (auto& scale : scales)
    {
        output.writePropertyName("scale");
        output.write(1, &scale.time);
        output.write(1, &scale.value);
        output.writeEndOfLine();
    }
}

template<typename T, typename V>
bool sample(double time, const T& values, V& value)
{
    if (values.size() == 0) return false;

    if (values.size() == 1)
    {
        value = values.front().value;
        return true;
    }

    auto pos_itr = values.begin();
    if (time <= pos_itr->time)
    {
        value = pos_itr->value;
        return true;
    }
    else
    {
        while (pos_itr != values.end() && pos_itr->time < time) ++pos_itr;

        auto before_pos_itr = pos_itr - 1;

        if (pos_itr != values.end())
        {
            double delta_time = (pos_itr->time - before_pos_itr->time);
            double r = delta_time != 0.0 ? (time - before_pos_itr->time) / delta_time : 0.5;

            value = mix(before_pos_itr->value, pos_itr->value, r);

            return true;
        }
        else
        {
            value = before_pos_itr->value;
            return true;
        }
    }
}

void TransformKeyframes::update(double time)
{
    dvec3 position;
    sample(time, positions, position);

    dquat rotation;
    sample(time, rotations, rotation);

    dvec3 scale(1.0, 1.0, 1.0);
    sample(time, scales, scale);

    matrix->set(vsg::scale(scale) * vsg::rotate(rotation) * translate(position));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TransformSampler
//
TransformSampler::TransformSampler() :
    position(0.0, 0.0, 0.0),
    rotation(),
    scale(1.0, 1.0, 1.0)
{
}

void TransformSampler::update(double time)
{
    if (keyFrames)
    {
        sample(time, keyFrames->positions, position);
        sample(time, keyFrames->rotations, rotation);
        sample(time, keyFrames->scales, scale);
    }

    if (object) object->accept(*this);
}

void TransformSampler::apply(mat4Value& matrix)
{
    matrix.set(mat4(vsg::scale(scale) * vsg::rotate(rotation) * translate(position)));
}

void TransformSampler::apply(dmat4Value& matrix)
{
    matrix.set(vsg::scale(scale) * vsg::rotate(rotation) * translate(position));
}

void TransformSampler::apply(MatrixTransform& mt)
{
    mt.matrix.set(vsg::scale(scale) * vsg::rotate(rotation) * translate(position));
}

void TransformSampler::apply(AnimationTransform& at)
{
    at.matrix->set(vsg::scale(scale) * vsg::rotate(rotation) * translate(position));
}

void TransformSampler::apply(RiggedTransform& rt)
{
    rt.matrix->set(vsg::scale(scale) * vsg::rotate(rotation) * translate(position));
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
    for (auto& keyframe : keyframes)
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
    for (auto& keyframe : keyframes)
    {
        output.write("keyframe", keyframe.time);
        output.writeValues("values", keyframe.values);
        output.writeValues("weights", keyframe.weights);
    }
}

void MorphKeyframes::update(double simulationTime)
{
    vsg::info("TODO  MorphKeyframes::update(", simulationTime, ") name = ", name);
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

void Animation::update(double simulationTime)
{
    double maxTime = 0.0;
    for (auto& tfk : transformKeyframes)
    {
        if (!tfk->positions.empty()) maxTime = std::max(maxTime, tfk->positions.back().time);
        if (!tfk->rotations.empty()) maxTime = std::max(maxTime, tfk->rotations.back().time);
        if (!tfk->scales.empty()) maxTime = std::max(maxTime, tfk->scales.back().time);
    }

    for (auto& mkf : morphKeyframes)
    {
        if (!mkf->keyframes.empty()) maxTime = std::max(maxTime, mkf->keyframes.back().time);
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

    for (auto tkf : transformKeyframes)
    {
        tkf->update(time);
    }
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

void AnimationGroup::update(double simulationTime)
{
    for (auto animation : active)
    {
        animation->update(simulationTime);
    }
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
