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

    class VSG_DECLSPEC CameraKeyframes : public Inherit<Object, CameraKeyframes>
    {
    public:
        CameraKeyframes();

        /// name of node
        std::string name;

        /// position key frames
        std::vector<VectorKey> origins;

        /// position key frames
        std::vector<VectorKey> positions;

        /// rotation key frames
        std::vector<QuatKey> rotations;

        /// field of view key frames
        std::vector<VectorKey> projections;

        void clear()
        {
            origins.clear();
            positions.clear();
            rotations.clear();
            projections.clear();
        }

        void add(double time, const dvec3& origin, const dvec3& position, const dquat& rotation, const dvec3& projection)
        {
            origins.push_back(VectorKey{time, origin});
            positions.push_back(VectorKey{time, position});
            rotations.push_back(QuatKey{time, rotation});
            projections.push_back(VectorKey{time, projection});
        }

        void add(double time, const dvec3& position, const dquat& rotation, const dvec3& projection)
        {
            positions.push_back(VectorKey{time, position});
            rotations.push_back(QuatKey{time, rotation});
            projections.push_back(VectorKey{time, projection});
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
        dvec3 projection;

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
