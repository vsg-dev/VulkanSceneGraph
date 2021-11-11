#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ConstVisitor.h>
#include <vsg/maths/mat3.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/quat.h>
#include <vsg/maths/vec3.h>

#include <cmath>

namespace vsg
{
    constexpr float PIf = 3.14159265358979323846f;
    constexpr double PI = 3.14159265358979323846;

    /// convert degrees to radians
    constexpr float radians(float degrees) noexcept { return degrees * (PIf / 180.0f); }
    constexpr double radians(double degrees) noexcept { return degrees * (PI / 180.0); }

    /// convert radians to degrees
    constexpr float degrees(float radians) noexcept { return radians * (180.0f / PIf); }
    constexpr double degrees(double radians) noexcept { return radians * (180.0 / PI); }

    /// Hermite interpolation between edge0 and edge1
    template<typename T>
    T smoothstep(T edge0, T edge1, T x)
    {
        if (x <= edge0)
            return edge0;
        else if (x >= edge1)
            return edge1;
        double r = (x - edge0) / (edge1 - edge0);
        return edge0 + (r * r * (3.0 - 2.0 * r)) * (edge1 - edge0);
    }

    /// Hermite interpolation between 0.0 and 1.0
    template<typename T>
    T smoothstep(T r)
    {
        if (r <= 0.0)
            return 0.0;
        else if (r >= 1.0)
            return 1.0;
        return r * r * (3.0 - 2.0 * r);
    }

    template<typename T>
    T mix(T start, T end, T r)
    {
        T one_minus_r = 1.0 - r;
        return start * one_minus_r + end * r;
    }

    template<typename T>
    constexpr t_mat4<T> rotate(const t_quat<T>& q)
    {
        T qxx(q.x * q.x);
        T qyy(q.y * q.y);
        T qzz(q.z * q.z);
        T qxy(q.x * q.y);
        T qxz(q.x * q.z);
        T qyz(q.y * q.z);
        T qwx(q.w * q.x);
        T qwy(q.w * q.y);
        T qwz(q.w * q.z);

        T zero(0.0);
        T one(1.0);
        T two(2.0);

        return t_mat4<T>(one - two * (qyy + qzz), two * (qxy + qwz), two * (qxz - qwy), zero,
                         two * (qxy - qwz), one - two * (qxx + qzz), two * (qyz + qwx), zero,
                         two * (qxz + qwy), two * (qyz - qwx), one - two * (qxx + qyy), zero,
                         zero, zero, zero, 1.0);
    }

    template<typename T>
    t_mat4<T> rotate(T angle_radians, T x, T y, T z)
    {
        const T c = std::cos(angle_radians);
        const T s = std::sin(angle_radians);
        const T one_minus_c = 1 - c;
        return t_mat4<T>(x * x * one_minus_c + c, y * x * one_minus_c + z * s, x * z * one_minus_c - y * s, 0,
                         x * y * one_minus_c - z * s, y * y * one_minus_c + c, y * z * one_minus_c + x * s, 0,
                         x * z * one_minus_c + y * s, y * z * one_minus_c - x * s, z * z * one_minus_c + c, 0,
                         0, 0, 0, 1);
    }

    template<typename T>
    t_mat4<T> rotate(T angle_radians, const t_vec3<T>& v)
    {
        return rotate(angle_radians, v.value[0], v.value[1], v.value[2]);
    }

    template<typename T>
    constexpr t_mat4<T> translate(T x, T y, T z)
    {
        return t_mat4<T>(1, 0, 0, 0,
                         0, 1, 0, 0,
                         0, 0, 1, 0,
                         x, y, z, 1);
    }

    template<typename T>
    constexpr t_mat4<T> translate(const t_vec3<T>& v)
    {
        return translate(v.value[0], v.value[1], v.value[2]);
    }

    template<typename T>
    constexpr t_mat4<T> scale(T sx, T sy, T sz)
    {
        return t_mat4<T>(sx, 0, 0, 0,
                         0, sy, 0, 0,
                         0, 0, sz, 0,
                         0, 0, 0, 1);
    }

    template<typename T>
    constexpr t_mat4<T> scale(const t_vec3<T>& v)
    {
        return scale(v.value[0], v.value[1], v.value[2]);
    }

    template<typename T>
    constexpr t_mat4<T> transpose(const t_mat4<T>& m)
    {
        return t_mat4<T>(m[0][0], m[1][0], m[2][0], m[3][0],
                         m[0][1], m[1][1], m[2][1], m[3][1],
                         m[0][2], m[1][2], m[2][2], m[3][2],
                         m[0][3], m[1][3], m[2][3], m[3][3]);
    }

    // Vulkan style 0 to 1 depth range
    template<typename T>
    constexpr t_mat4<T> perspective(T fovy_radians, T aspectRatio, T zNear, T zFar)
    {
        T f = static_cast<T>(1.0 / std::tan(fovy_radians * 0.5));
        T r = static_cast<T>(1.0 / (zNear - zFar));
        return t_mat4<T>(f / aspectRatio, 0, 0, 0,
                         0, -f, 0, 0,
                         0, 0, (zFar)*r, -1,
                         0, 0, (zFar * zNear) * r, 0);
    }

    // from vulkan cookbook
    template<typename T>
    constexpr t_mat4<T> orthographic(T left, T right, T bottom, T top, T zNear, T zFar)
    {
        return t_mat4<T>(2.0 / (right - left), 0.0, 0.0, 0.0,
                         0.0, 2.0 / (bottom - top), 0.0, 0.0,
                         0.0, 0.0, 1.0 / (zNear - zFar), 0.0,
                         -(right + left) / (right - left), -(bottom + top) / (bottom - top), zNear / (zNear - zFar), 1.0);
    }

    template<typename T>
    constexpr t_mat4<T> lookAt(const t_vec3<T>& eye, const t_vec3<T>& center, const t_vec3<T>& up)
    {
        using vec_type = t_vec3<T>;

        vec_type forward = normalize(center - eye);
        vec_type up_normal = normalize(up);
        vec_type side = normalize(cross(forward, up_normal));
        vec_type u = normalize(cross(side, forward));

        return t_mat4<T>(side[0], u[0], -forward[0], 0,
                         side[1], u[1], -forward[1], 0,
                         side[2], u[2], -forward[2], 0,
                         0, 0, 0, 1) *
               vsg::translate(-eye.x, -eye.y, -eye.z);
    }

    /// Hint on axis, using Collada conventions, all Right Hand
    enum class CoordinateConvention
    {
        NO_PREFERENCE,
        X_UP, // x up, y left/west, z out/south
        Y_UP, // x right/east, y up, z out/south
        Z_UP  // x right/east, y forward/north, z up
    };

    /// compute the transformation matrix required to transform from one coordinate frame convention to another.
    /// return true if required and matrix modified, return false if no transformation is required.
    extern VSG_DECLSPEC bool transform(CoordinateConvention source, CoordinateConvention destination, dmat4& matrix);

    /// fast float matrix inversion that use assumes the matrix is composed of only scales, rotations and translations forming a 4x3 matrix.
    extern VSG_DECLSPEC mat4 inverse_4x3(const mat4& m);

    /// fast double matrix inversion that use assumes the matrix is composed of only scales, rotations and translations forming a 4x3 matrix.
    extern VSG_DECLSPEC dmat4 inverse_4x3(const dmat4& m);

    /// general purpose 4x4 float matrix inversion.
    extern VSG_DECLSPEC mat4 inverse_4x4(const mat4& m);

    /// general purpose 4x4 float matrix inversion.
    extern VSG_DECLSPEC dmat4 inverse_4x4(const dmat4& m);

    /// matrix float inversion with automatic selection of inverse_4x3 when appropriate, otherwise uses inverse_4x4
    extern VSG_DECLSPEC mat4 inverse(const mat4& m);

    /// double matrix inversion with automatic selection of inverse_4x3 when appropriate, otherwise uses inverse_4x4
    extern VSG_DECLSPEC dmat4 inverse(const dmat4& m);

    /// compute the bounding sphere that encloses a frustum defined by specified float ModelViewMatrixProjection
    extern VSG_DECLSPEC sphere computeFrustumBound(const mat4& m);

    /// compute the bounding sphere that encloses a frustum defined by specified double ModelViewMatrixProjection
    extern VSG_DECLSPEC dsphere computeFrustumBound(const dmat4& m);

    struct ComputeTransform : public ConstVisitor
    {
        dmat4 matrix;

        void apply(const Transform& transform) override;
        void apply(const MatrixTransform& mt) override;
    };

    // convinience function for accumulating the transforms in scene graph along a specified nodePath.
    template<typename T>
    dmat4 computeTransform(const T& nodePath)
    {
        ComputeTransform ct;
        for (auto& node : nodePath)
        {
            node->accept(ct);
        }
        return ct.matrix;
    }

} // namespace vsg
