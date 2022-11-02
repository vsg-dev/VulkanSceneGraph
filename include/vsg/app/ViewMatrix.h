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

    /// ViewMatrix is a base class for specifying the Camera view matrix and it's inverse.
    class VSG_DECLSPEC ViewMatrix : public Inherit<Object, ViewMatrix>
    {
    public:
        virtual dmat4 transform() const = 0;

        virtual dmat4 inverse() const
        {
            return vsg::inverse(transform());
        }
    };
    VSG_type_name(vsg::ViewMatrix);

    /// LookAt is a ViewMatrix that implements gluLookAt model for specify the view matrix.
    class VSG_DECLSPEC LookAt : public Inherit<ViewMatrix, LookAt>
    {
    public:
        LookAt() :
            eye(0.0, 0.0, 0.0),
            center(0.0, 1.0, 0.0),
            up(0.0, 0.0, 1.0)
        {
        }

        LookAt(const LookAt& lookAt) :
            Inherit(lookAt),
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

        void transform(const dmat4& matrix)
        {
            up = normalize(matrix * (eye + up) - matrix * eye);
            center = matrix * center;
            eye = matrix * eye;
        }

        void set(const dmat4& matrix)
        {
            up = normalize(matrix * (dvec3(0.0, 0.0, 0.0) + dvec3(0.0, 1.0, 0.0)) - matrix * dvec3(0.0, 0.0, 0.0));
            center = matrix * dvec3(0.0, 0.0, -1.0);
            eye = matrix * dvec3(0.0, 0.0, 0.0);
        }

        dmat4 transform() const override { return lookAt(eye, center, up); }

        void read(Input& input) override;
        void write(Output& output) const override;

        dvec3 eye;
        dvec3 center;
        dvec3 up;
    };
    VSG_type_name(vsg::LookAt);

    /// RelativeViewMatrix is a ViewMatrix that decorates another ViewMatrix and pre-multiplies it's transform matrix to give a relative view matrix.
    class RelativeViewMatrix : public Inherit<ViewMatrix, RelativeViewMatrix>
    {
    public:
        RelativeViewMatrix(const dmat4& m, ref_ptr<ViewMatrix> vm) :
            matrix(m),
            viewMatrix(vm)
        {
        }

        /// returns matrix * viewMatrix->transform()
        dmat4 transform() const override
        {
            return matrix * viewMatrix->transform();
        }

        dmat4 matrix;
        ref_ptr<ViewMatrix> viewMatrix;
    };
    VSG_type_name(vsg::RelativeViewMatrix);

    /// TrackingViewMatrix is a ViewMatrix that tracks an object in the scene graph.
    /// The view matrix is computed by accumulating the transform matrix encountered
    /// along a objectPath and then pre-multiplying this by specified matrix.
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
        dmat4 transform() const override;
        dmat4 inverse() const override;

        dmat4 matrix;
        RefObjectPath objectPath;
    };
    VSG_type_name(vsg::TrackingViewMatrix);

} // namespace vsg
