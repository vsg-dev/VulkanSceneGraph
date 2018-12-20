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

    // Base class for projection matrices
    class ProjectionMatrix : public Inherit<Object, ProjectionMatrix>
    {
    public:
        virtual void get(mat4& matrix) const = 0;
        virtual void get(dmat4& matrix) const = 0;
    };

    class Perspective : public Inherit<ProjectionMatrix, Perspective>
    {
    public:

        Perspective():
            fieldOfViewY(60.0),
            aspectRatio(1.0),
            nearDistance(1.0),
            farDistance(10000.0)
        {
        }

        Perspective(double fov, double ar, double nd, double fd):
            fieldOfViewY(fov),
            aspectRatio(ar),
            nearDistance(nd),
            farDistance(fd)
        {
        }

        void get(mat4& matrix) const override { matrix = perspective(radians(fieldOfViewY), aspectRatio, nearDistance, farDistance); }
        void get(dmat4& matrix) const override { matrix = perspective(radians(fieldOfViewY), aspectRatio, nearDistance, farDistance); }

        double fieldOfViewY;
        double aspectRatio;
        double nearDistance;
        double farDistance;
    };

    class ViewMatrix : public Inherit<Object, ViewMatrix>
    {
    public:
        virtual void get(mat4& matrix) const = 0;
        virtual void get(dmat4& matrix) const = 0;
    };

    class LookAt : public Inherit<ViewMatrix, LookAt>
    {
    public:

        LookAt():
            eye(0.0, 0.0, 0.0),
            center(0.0, 1.0, 0.0),
            up(0.0, 0.0, 1.0)
        {
        }

        LookAt(const dvec3& in_eye, const dvec3& in_center, const dvec3& in_up):
            eye(in_eye),
            center(in_center),
            up(in_up)
        {
            dvec3 look = normalize(center-eye);
            dvec3 side = normalize(cross(look, up));
            up = normalize(cross(side, look));
        }

        void get(mat4& matrix) const override { matrix = lookAt(eye, center, up); }
        void get(dmat4& matrix) const override { matrix = lookAt(eye, center, up); }

        dvec3 eye;
        dvec3 center;
        dvec3 up;
    };


    class Camera : public Inherit<Object, Object>
    {
    public:

        Camera();

        Camera(ref_ptr<ProjectionMatrix> projectionMatrix, ref_ptr<ViewMatrix> viewMatrix);

        void setProjectionMatrix(ref_ptr<ProjectionMatrix> projectionMatrix) { _projectionMatrix = projectionMatrix; }
        ProjectionMatrix* getProjectionMatrix() const { return _projectionMatrix; }

        void setViewMatrix(ref_ptr<ViewMatrix> viewMatrix) { _viewMatrix = viewMatrix; }
        ViewMatrix* getViewMatrix() const { return _viewMatrix; }

    protected:

        ref_ptr<ProjectionMatrix> _projectionMatrix;
        ref_ptr<ViewMatrix> _viewMatrix;
    };
}
