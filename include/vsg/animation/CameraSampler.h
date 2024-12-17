#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/TransformSampler.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/maths/transform.h>

namespace vsg
{

    using time_path = time_value<RefObjectPath>;

    class VSG_DECLSPEC CameraKeyframes : public Inherit<Object, CameraKeyframes>
    {
    public:
        CameraKeyframes();

        /// name of node
        std::string name;

        // object tracking key frames
        std::vector<time_path> tracking;

        /// position key frames
        std::vector<time_dvec3> origins;

        /// position key frames
        std::vector<time_dvec3> positions;

        /// rotation key frames
        std::vector<time_dquat> rotations;

        /// field of view key frames
        std::vector<time_double> fieldOfViews;

        /// near/far key frames
        std::vector<time_dvec2> nearFars;

        void clear()
        {
            origins.clear();
            positions.clear();
            rotations.clear();
            fieldOfViews.clear();
            nearFars.clear();
        }

        void add(double time, const dvec3& origin, const dvec3& position, const dquat& rotation, double fov, const dvec2& nearFar)
        {
            origins.push_back(VectorKey{time, origin});
            positions.push_back(VectorKey{time, position});
            rotations.push_back(QuatKey{time, rotation});
            fieldOfViews.push_back(time_double{time, fov});
            nearFars.push_back(time_dvec2{time, nearFar});
        }

        void add(double time, const dvec3& origin, const dvec3& position, const dquat& rotation, double fov)
        {
            origins.push_back(VectorKey{time, origin});
            positions.push_back(VectorKey{time, position});
            rotations.push_back(QuatKey{time, rotation});
            fieldOfViews.push_back(time_double{time, fov});
        }

        void add(double time, const dvec3& position, const dquat& rotation, double fov, const dvec2& nearFar)
        {
            positions.push_back(VectorKey{time, position});
            rotations.push_back(QuatKey{time, rotation});
            fieldOfViews.push_back(time_double{time, fov});
            nearFars.push_back(time_dvec2{time, nearFar});
        }

        void add(double time, const dvec3& position, const dquat& rotation, double fov)
        {
            positions.push_back(VectorKey{time, position});
            rotations.push_back(QuatKey{time, rotation});
            fieldOfViews.push_back(time_double{time, fov});
        }

        void add(double time, const dvec3& origin, const dvec3& position, const dquat& rotation)
        {
            origins.push_back(VectorKey{time, origin});
            positions.push_back(VectorKey{time, position});
            rotations.push_back(QuatKey{time, rotation});
        }

        void add(double time, const dvec3& position, const dquat& rotation)
        {
            positions.push_back(VectorKey{time, position});
            rotations.push_back(QuatKey{time, rotation});
        }

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::CameraKeyframes);

    /// Animation sampler for sampling position, rotation and scale keyframes for setting camera view and project matrices.
    class VSG_DECLSPEC CameraSampler : public Inherit<AnimationSampler, CameraSampler>
    {
    public:
        CameraSampler();
        CameraSampler(const CameraSampler& rhs, const CopyOp& copyop = {});

        ref_ptr<CameraKeyframes> keyframes;
        ref_ptr<Object> object;

        // updated using keyFrames
        dvec3 origin;
        dvec3 position;
        dquat rotation;
        double fieldOfView;
        dvec2 nearFar;

        void update(double time) override;
        double maxTime() const override;

        inline dmat4 transform() const { return translate(position) * vsg::rotate(rotation); }

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return CameraSampler::create(*this, copyop); }
        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        void apply(mat4Value& mat) override;
        void apply(dmat4Value& mat) override;
        void apply(LookAt& lookAt) override;
        void apply(LookDirection& lookDirection) override;
        void apply(Perspective& perspective) override;
        void apply(Camera& camera) override;
    };
    VSG_type_name(vsg::CameraSampler);

} // namespace vsg
