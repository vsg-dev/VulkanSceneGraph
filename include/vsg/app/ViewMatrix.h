#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>
#include <vsg/maths/transform.h>

namespace vsg
{

    /// ViewMatrix is a base class for specifying the Camera view matrix and its inverse.
    class VSG_DECLSPEC ViewMatrix : public Inherit<Object, ViewMatrix>
    {
    public:
        ViewMatrix()
        {
        }

        explicit ViewMatrix(const ViewMatrix& vm, const CopyOp& copyop = {}) :
            Inherit(vm, copyop)
        {
        }

        /// origin value provides a means of translating the view matrix relative to the origin of any CoordinateFrame subgraphs
        /// to maximize the precision when moving around the CoordinateFrame subgraph.  This is helpful for astronmically large
        /// scenes where standard double precision is insufficient for avoiding visually significant numerical errors.
        dvec3 origin;

        virtual dmat4 transform(const dvec3& offset = {}) const = 0;

        virtual dmat4 inverse(const dvec3& offset = {}) const
        {
            return vsg::inverse(transform(offset));
        }

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::ViewMatrix);

    /// LookAt is a ViewMatrix that implements the gluLookAt model for specifying the view matrix.
    class VSG_DECLSPEC LookAt : public Inherit<ViewMatrix, LookAt>
    {
    public:
        LookAt() :
            eye(0.0, 0.0, 0.0),
            center(0.0, 1.0, 0.0),
            up(0.0, 0.0, 1.0)
        {
        }

        LookAt(const LookAt& lookAt, const CopyOp& copyop = {}) :
            Inherit(lookAt, copyop),
            eye(lookAt.eye),
            center(lookAt.center),
            up(lookAt.up)
        {
        }

        LookAt(const dvec3& in_eye, const dvec3& in_center, const dvec3& in_up) :
            eye(in_eye),
            center(in_center),
            up(in_up)
        {
            dvec3 look = normalize(center - eye);
            dvec3 side = normalize(cross(look, up));
            up = normalize(cross(side, look));
        }

        LookAt& operator=(const LookAt& lookAt)
        {
            eye = lookAt.eye;
            center = lookAt.center;
            up = lookAt.up;
            return *this;
        }

        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return LookAt::create(*this, copyop); }

        void transform(const dmat4& matrix);

        void set(const dmat4& matrix);

        dmat4 transform(const dvec3& offset = {}) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        dvec3 eye;
        dvec3 center;
        dvec3 up;
    };
    VSG_type_name(vsg::LookAt);

    /// LookDirection is a ViewMatrix that uses a position and rotation to set the view matrix.
    class VSG_DECLSPEC LookDirection : public vsg::Inherit<ViewMatrix, LookDirection>
    {
    public:
        LookDirection() :
            position(0.0, 0.0, 0.0),
            rotation()
        {
        }

        LookDirection(const LookDirection& view, const CopyOp& copyop = {}) :
            Inherit(view, copyop),
            position(view.position),
            rotation(view.rotation)
        {
        }

        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return LookDirection::create(*this, copyop); }

        dvec3 position;
        dquat rotation;

        void set(const dmat4& matrix);

        dmat4 transform(const dvec3& offset = {}) const override;
    };
    VSG_type_name(vsg::LookDirection);

    /// RelativeViewMatrix is a ViewMatrix that decorates another ViewMatrix and pre-multiplies its transform matrix to give a relative view matrix.
    class VSG_DECLSPEC RelativeViewMatrix : public Inherit<ViewMatrix, RelativeViewMatrix>
    {
    public:
        RelativeViewMatrix(const dmat4& m, ref_ptr<ViewMatrix> vm) :
            matrix(m),
            viewMatrix(vm)
        {
        }

        /// returns matrix * viewMatrix->transform()
        dmat4 transform(const dvec3& offset = {}) const override;

        dmat4 matrix;
        ref_ptr<ViewMatrix> viewMatrix;
    };
    VSG_type_name(vsg::RelativeViewMatrix);

    /// TrackingViewMatrix is a ViewMatrix that tracks an object in the scene graph.
    /// The view matrix is computed by accumulating the transform matrix encountered
    /// along an objectPath and then pre-multiplying this by the specified matrix.
    class VSG_DECLSPEC TrackingViewMatrix : public Inherit<ViewMatrix, TrackingViewMatrix>
    {
    public:
        template<typename T>
        explicit TrackingViewMatrix(const dmat4& initial_matrix, const T& path) :
            matrix(initial_matrix),
            objectPath(path.begin(), path.end()) {}

        template<typename T>
        explicit TrackingViewMatrix(const T& path) :
            objectPath(path.begin(), path.end()) {}

        /// returns matrix * computeTransfrom(objectPath)
        dmat4 transform(const dvec3& offset = {}) const override;
        dmat4 inverse(const dvec3& offset = {}) const override;

        dmat4 matrix;
        RefObjectPath objectPath;
    };
    VSG_type_name(vsg::TrackingViewMatrix);

} // namespace vsg
