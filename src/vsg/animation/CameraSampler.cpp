/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/AnimationGroup.h>
#include <vsg/animation/CameraSampler.h>
#include <vsg/animation/Joint.h>
#include <vsg/app/Camera.h>
#include <vsg/core/compare.h>
#include <vsg/io/Input.h>
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

    // read tracking key frames
    uint32_t num_tracking = input.readValue<uint32_t>("tracking");
    tracking.resize(num_tracking);
    for (auto& track : tracking)
    {
        input.matchPropertyName("track");
        input.read(1, &track.time);
        input.readObjects("path", track.value);
    }

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

    // read field of view key frames
    uint32_t num_fieldOfViews = input.readValue<uint32_t>("fieldOfViews");
    fieldOfViews.resize(num_fieldOfViews);
    for (auto& fov : fieldOfViews)
    {
        input.matchPropertyName("fov");
        input.read(1, &fov.time);
        input.read(1, &fov.value);
    }

    // read near/far key frames
    uint32_t num_nearFars = input.readValue<uint32_t>("nearFars");
    nearFars.resize(num_nearFars);
    for (auto& nf : nearFars)
    {
        input.matchPropertyName("nearfar");
        input.read(1, &nf.time);
        input.read(1, &nf.value);
    }
}

void CameraKeyframes::write(Output& output) const
{
    Object::write(output);

    output.write("name", name);

    // write position key frames
    output.writeValue<uint32_t>("tracking", tracking.size());
    for (const auto& track : tracking)
    {
        output.writePropertyName("track");
        output.write(1, &track.time);
        output.writeEndOfLine();

        output.writeObjects("path", track.value);
    }

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
    for (const auto& scale : fieldOfViews)
    {
        output.writePropertyName("fov");
        output.write(1, &scale.time);
        output.write(1, &scale.value);
        output.writeEndOfLine();
    }

    // write field of view key frames
    output.writeValue<uint32_t>("fieldOfViews", fieldOfViews.size());
    for (const auto& fov : fieldOfViews)
    {
        output.writePropertyName("fov");
        output.write(1, &fov.time);
        output.write(1, &fov.value);
        output.writeEndOfLine();
    }

    // read near/far key frames
    output.writeValue<uint32_t>("nearFars", nearFars.size());
    for (const auto& nf : nearFars)
    {
        output.writePropertyName("nearfar");
        output.write(1, &nf.time);
        output.write(1, &nf.value);
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

        auto find_values = [](const RefObjectPath& path, dvec3& in_origin, dvec3& in_position, dquat& in_rotation) -> void {
            ComputeTransform ct;
            for (auto& obj : path) obj->accept(ct);

            in_origin = ct.origin;

            dvec3 scale;
            vsg::decompose(ct.matrix, in_position, in_rotation, scale);
        };

        auto& tracking = keyframes->tracking;
        if (tracking.size() == 1)
        {
            find_values(tracking.front().value, origin, position, rotation);
        }
        else if (!tracking.empty())
        {
            auto pos_itr = std::lower_bound(tracking.begin(), tracking.end(), time, [](const time_path& elem, double t) -> bool { return elem.time < t; });
            if (pos_itr == tracking.begin())
            {
                find_values(tracking.front().value, origin, position, rotation);
            }
            else if (pos_itr == tracking.end())
            {
                find_values(tracking.back().value, origin, position, rotation);
            }
            else
            {
                auto before_pos_itr = pos_itr - 1;
                double delta_time = (pos_itr->time - before_pos_itr->time);
                double r = delta_time != 0.0 ? (time - before_pos_itr->time) / delta_time : 0.5;

                dvec3 origin_before, position_before;
                dquat rotation_before;
                find_values(before_pos_itr->value, origin_before, position_before, rotation_before);

                dvec3 origin_after, position_after;
                dquat rotation_after;
                find_values(pos_itr->value, origin_after, position_after, rotation_after);

                // convert origin input values and ratio to long double to minimize the intermediate rounding errors.
                origin = mix(ldvec3(origin_before), ldvec3(origin_after), static_cast<long double>(r));

                position = mix(position_before, position_after, r);
                rotation = mix(rotation_before, rotation_after, r);
            }
        }
    }

    if (object) object->accept(*this);
}

double CameraSampler::maxTime() const
{
    double maxTime = 0.0;
    if (keyframes)
    {
        if (!keyframes->tracking.empty()) maxTime = std::max(maxTime, keyframes->tracking.back().time);
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
        bool has_tracking = !keyframes->tracking.empty();
        if (!keyframes->origins.empty() || has_tracking) lookAt.origin = origin;
        if (!keyframes->positions.empty() || !keyframes->rotations.empty() || has_tracking)
        {
            lookAt.set(transform());
        }
    }
}

void CameraSampler::apply(LookDirection& lookDirection)
{
    if (keyframes)
    {
        bool has_tracking = !keyframes->tracking.empty();
        if (!keyframes->origins.empty() || has_tracking) lookDirection.origin = origin;
        if (!keyframes->positions.empty() || has_tracking) lookDirection.position = position;
        if (!keyframes->rotations.empty() || has_tracking) lookDirection.rotation = rotation;
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
