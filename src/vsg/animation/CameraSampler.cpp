/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/AnimationGroup.h>
#include <vsg/animation/Joint.h>
#include <vsg/animation/CameraSampler.h>
#include <vsg/app/Camera.h>
#include <vsg/core/compare.h>
#include <vsg/io/Input.h>
#include <vsg/io/Options.h>
#include <vsg/io/Output.h>
#include <vsg/nodes/MatrixTransform.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CameraKeyframes
//
CameraKeyframes::CameraKeyframes()
{
}

void CameraKeyframes::read(Input& input)
{
    Object::read(input);

    input.read("name", name);

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
    uint32_t num_fieldOfViews = input.readValue<uint32_t>("fieldOfViews");
    fieldOfViews.resize(num_fieldOfViews);
    for (auto& scale : fieldOfViews)
    {
        input.matchPropertyName("projection");
        input.read(1, &scale.time);
        input.read(1, &scale.value);
    }
}

void CameraKeyframes::write(Output& output) const
{
    Object::write(output);

    output.write("name", name);

    // write position key frames
    output.writeValue<uint32_t>("positions", positions.size());
    for (const auto& position : positions)
    {
        output.writePropertyName("position");
        output.write(1, &position.time);
        output.write(1, &position.value);
        output.writeEndOfLine();
    }

    // write rotation key frames
    output.writeValue<uint32_t>("rotations", rotations.size());
    for (const auto& rotation : rotations)
    {
        output.writePropertyName("rotation");
        output.write(1, &rotation.time);
        output.write(1, &rotation.value);
        output.writeEndOfLine();
    }

    // write scale key frames
    output.writeValue<uint32_t>("fieldOfViews", fieldOfViews.size());
    for (const auto& scale : fieldOfViews)
    {
        output.writePropertyName("projection");
        output.write(1, &scale.time);
        output.write(1, &scale.value);
        output.writeEndOfLine();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CameraSampler
//
CameraSampler::CameraSampler() :
    origin(0.0, 0.0, 0.0),
    position(0.0, 0.0, 0.0),
    rotation(),
    fieldOfView(60.0),
    nearFar(1.0, 1e10)
{
}

CameraSampler::CameraSampler(const CameraSampler& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    keyframes(copyop(rhs.keyframes)),
    object(copyop(rhs.object)),
    position(rhs.position),
    rotation(rhs.rotation),
    fieldOfView(rhs.fieldOfView),
    nearFar(rhs.nearFar)
{
}

int CameraSampler::compare(const Object& rhs_object) const
{
    int result = AnimationSampler::compare(rhs_object);
    if (result != 0) return result;

    const auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_pointer(keyframes, rhs.keyframes)) != 0) return result;
    return compare_pointer(object, rhs.object);
}

void CameraSampler::update(double time)
{
    if (keyframes)
    {
        sample(time, keyframes->origins, origin);
        sample(time, keyframes->positions, position);
        sample(time, keyframes->rotations, rotation);
        sample(time, keyframes->fieldOfViews, fieldOfView);
        sample(time, keyframes->nearFars, nearFar);
    }

    if (object) object->accept(*this);
}

double CameraSampler::maxTime() const
{
    double maxTime = 0.0;
    if (keyframes)
    {
        if (!keyframes->origins.empty()) maxTime = std::max(maxTime, keyframes->origins.back().time);
        if (!keyframes->positions.empty()) maxTime = std::max(maxTime, keyframes->positions.back().time);
        if (!keyframes->rotations.empty()) maxTime = std::max(maxTime, keyframes->rotations.back().time);
        if (!keyframes->fieldOfViews.empty()) maxTime = std::max(maxTime, keyframes->fieldOfViews.back().time);
        if (!keyframes->nearFars.empty()) maxTime = std::max(maxTime, keyframes->nearFars.back().time);
    }

    return maxTime;
}

void CameraSampler::apply(mat4Value& matrix)
{
    matrix.set(mat4(transform()));
}

void CameraSampler::apply(dmat4Value& matrix)
{
    matrix.set(transform());
}

void CameraSampler::apply(LookAt& lookAt)
{
    if (keyframes)
    {
        if (!keyframes->origins.empty()) lookAt.origin = origin;
        if (!keyframes->positions.empty() || !keyframes->rotations.empty())
        {
            lookAt.set(transform());
        }
    }
}

void CameraSampler::apply(LookDirection& lookDirection)
{
    if (keyframes)
    {
        if (!keyframes->origins.empty()) lookDirection.origin = origin;
        if (!keyframes->positions.empty()) lookDirection.position = position;
        if (!keyframes->rotations.empty()) lookDirection.rotation = rotation;
    }
}

void CameraSampler::apply(Perspective& perspective)
{
    if (keyframes && !keyframes->fieldOfViews.empty())
    {
        perspective.fieldOfViewY = fieldOfView;
    }
    if (keyframes && !keyframes->nearFars.empty())
    {
        perspective.nearDistance = nearFar[0];
        perspective.farDistance = nearFar[1];
    }
}

void CameraSampler::apply(Camera& camera)
{
    if (camera.projectionMatrix) camera.projectionMatrix->accept(*this);
    if (camera.viewMatrix) camera.viewMatrix->accept(*this);
}

void CameraSampler::read(Input& input)
{
    AnimationSampler::read(input);
    input.read("keyframes", keyframes);
    input.read("object", object);
}

void CameraSampler::write(Output& output) const
{
    AnimationSampler::write(output);
    output.write("keyframes", keyframes);
    output.write("object", object);
}
