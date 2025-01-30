/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/Camera.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/CoordinateFrame.h>
#include <vsg/nodes/MatrixTransform.h>

using namespace vsg;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// inverse
//
template<class T>
typename T::value_type difference(const T& lhs, const T& rhs)
{
    typename T::value_type delta = 0.0;
    for (std::size_t c = 0; c < lhs.columns(); ++c)
    {
        for (std::size_t r = 0; r < rhs.rows(); ++r)
        {
            delta += std::abs(lhs[c][r] - rhs[c][r]);
        }
    }
    return delta;
}

template<class T>
t_mat3<T> t_inverse_3x3(const t_mat4<T>& m)
{
    using value_type = T;

    value_type det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

    if (det == value_type(0.0)) return t_mat3<T>(std::numeric_limits<value_type>::quiet_NaN()); // could use signaling_NaN()

    value_type inv_det = value_type(1.0) / det;

    value_type m00 = inv_det * (m[1][1] * m[2][2] - m[1][2] * m[2][1]);
    value_type m01 = inv_det * (m[0][2] * m[2][1] - m[0][1] * m[2][2]);
    value_type m02 = inv_det * (m[0][1] * m[1][2] - m[0][2] * m[1][1]);
    value_type m10 = inv_det * (m[1][2] * m[2][0] - m[1][0] * m[2][2]);
    value_type m11 = inv_det * (m[0][0] * m[2][2] - m[0][2] * m[2][0]);
    value_type m12 = inv_det * (m[0][2] * m[1][0] - m[0][0] * m[1][2]);
    value_type m20 = inv_det * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    value_type m21 = inv_det * (m[0][1] * m[2][0] - m[0][0] * m[2][1]);
    value_type m22 = inv_det * (m[0][0] * m[1][1] - m[0][1] * m[1][0]);

    return t_mat3<T>(m00, m01, m02,  // column 0
                     m10, m11, m12,  // column 1
                     m20, m21, m22); // column 2
}

mat3 vsg::inverse_3x3(const mat4& m)
{
    return t_inverse_3x3<float>(m);
}

dmat3 vsg::inverse_3x3(const dmat4& m)
{
    return t_inverse_3x3<double>(m);
}

template<class T>
T t_inverse_4x3(const T& m)
{
    using value_type = typename T::value_type;

    value_type det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

    if (det == value_type(0.0)) return T(std::numeric_limits<value_type>::quiet_NaN()); // could use signaling_NaN()

    value_type A1223 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
    value_type A0223 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
    value_type A0123 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
    value_type A1213 = m[1][1] * m[3][2] - m[1][2] * m[3][1];
    value_type A0213 = m[1][0] * m[3][2] - m[1][2] * m[3][0];
    value_type A0113 = m[1][0] * m[3][1] - m[1][1] * m[3][0];

    value_type inv_det = value_type(1.0) / det;

    value_type m00 = inv_det * (m[1][1] * m[2][2] - m[1][2] * m[2][1]);
    value_type m01 = inv_det * (m[0][2] * m[2][1] - m[0][1] * m[2][2]);
    value_type m02 = inv_det * (m[0][1] * m[1][2] - m[0][2] * m[1][1]);
    value_type m10 = inv_det * (m[1][2] * m[2][0] - m[1][0] * m[2][2]);
    value_type m11 = inv_det * (m[0][0] * m[2][2] - m[0][2] * m[2][0]);
    value_type m12 = inv_det * (m[0][2] * m[1][0] - m[0][0] * m[1][2]);
    value_type m20 = inv_det * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    value_type m21 = inv_det * (m[0][1] * m[2][0] - m[0][0] * m[2][1]);
    value_type m22 = inv_det * (m[0][0] * m[1][1] - m[0][1] * m[1][0]);
    value_type m30 = inv_det * (m[1][1] * A0223 - m[1][2] * A0123 - m[1][0] * A1223);
    value_type m31 = inv_det * (m[0][0] * A1223 - m[0][1] * A0223 + m[0][2] * A0123);
    value_type m32 = inv_det * (m[0][1] * A0213 - m[0][2] * A0113 - m[0][0] * A1213);

    return T(m00, m01, m02, value_type(0.0),  // column 0
             m10, m11, m12, value_type(0.0),  // column 1
             m20, m21, m22, value_type(0.0),  // column 2
             m30, m31, m32, value_type(1.0)); // column 3
}

template<class T>
T t_inverse_4x4(const T& m)
{
    using value_type = typename T::value_type;

    value_type A2323 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
    value_type A1323 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
    value_type A1223 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
    value_type A0323 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
    value_type A0223 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
    value_type A0123 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
    value_type A2313 = m[1][2] * m[3][3] - m[1][3] * m[3][2];
    value_type A1313 = m[1][1] * m[3][3] - m[1][3] * m[3][1];
    value_type A1213 = m[1][1] * m[3][2] - m[1][2] * m[3][1];
    value_type A2312 = m[1][2] * m[2][3] - m[1][3] * m[2][2];
    value_type A1312 = m[1][1] * m[2][3] - m[1][3] * m[2][1];
    value_type A1212 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    value_type A0313 = m[1][0] * m[3][3] - m[1][3] * m[3][0];
    value_type A0213 = m[1][0] * m[3][2] - m[1][2] * m[3][0];
    value_type A0312 = m[1][0] * m[2][3] - m[1][3] * m[2][0];
    value_type A0212 = m[1][0] * m[2][2] - m[1][2] * m[2][0];
    value_type A0113 = m[1][0] * m[3][1] - m[1][1] * m[3][0];
    value_type A0112 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

    value_type det = m[0][0] * (m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223) - m[0][1] * (m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223) + m[0][2] * (m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123) - m[0][3] * (m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123);

    if (det == value_type(0.0)) return T(std::numeric_limits<value_type>::quiet_NaN()); // could use signaling_NaN()

    value_type inv_det = value_type(1.0) / det;

    return T(
        inv_det * (m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223),  // 00
        inv_det * -(m[0][1] * A2323 - m[0][2] * A1323 + m[0][3] * A1223), // 01
        inv_det * (m[0][1] * A2313 - m[0][2] * A1313 + m[0][3] * A1213),  // 02
        inv_det * -(m[0][1] * A2312 - m[0][2] * A1312 + m[0][3] * A1212), // 03
        inv_det * -(m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223), // 10
        inv_det * (m[0][0] * A2323 - m[0][2] * A0323 + m[0][3] * A0223),  // 11
        inv_det * -(m[0][0] * A2313 - m[0][2] * A0313 + m[0][3] * A0213), // 12
        inv_det * (m[0][0] * A2312 - m[0][2] * A0312 + m[0][3] * A0212),  // 13
        inv_det * (m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123),  // 20
        inv_det * -(m[0][0] * A1323 - m[0][1] * A0323 + m[0][3] * A0123), // 21
        inv_det * (m[0][0] * A1313 - m[0][1] * A0313 + m[0][3] * A0113),  // 22
        inv_det * -(m[0][0] * A1312 - m[0][1] * A0312 + m[0][3] * A0112), // 23
        inv_det * -(m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123), // 30
        inv_det * (m[0][0] * A1223 - m[0][1] * A0223 + m[0][2] * A0123),  // 31
        inv_det * -(m[0][0] * A1213 - m[0][1] * A0213 + m[0][2] * A0113), // 32
        inv_det * (m[0][0] * A1212 - m[0][1] * A0212 + m[0][2] * A0112)); // 33
}

mat4 vsg::inverse_4x3(const mat4& m)
{
    return t_inverse_4x3(m);
}

mat4 vsg::inverse_4x4(const mat4& m)
{
    return t_inverse_4x4(m);
}

mat4 vsg::inverse(const mat4& m)
{
    if (m[0][3] == 0.0f && m[1][3] == 0.0f && m[2][3] == 0.0f && m[3][3] == 1.0f)
    {
        return t_inverse_4x3(m);
    }
    else
    {
        return t_inverse_4x4(m);
    }
}

dmat4 vsg::inverse_4x3(const dmat4& m)
{
    return t_inverse_4x3(m);
}

dmat4 vsg::inverse_4x4(const dmat4& m)
{
    return t_inverse_4x4(m);
}

dmat4 vsg::inverse(const dmat4& m)
{
    if (m[0][3] == 0.0 && m[1][3] == 0.0 && m[2][3] == 0.0 && m[3][3] == 1.0)
    {
        return t_inverse_4x3(m);
    }
    else
    {
        return t_inverse_4x4(m);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// compute determinant of a matrix
//
template<class T>
T t_determinant(const t_mat4<T>& m)
{
    using value_type = T;

    value_type A2323 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
    value_type A1323 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
    value_type A1223 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
    value_type A0323 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
    value_type A0223 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
    value_type A0123 = m[2][0] * m[3][1] - m[2][1] * m[3][0];

    value_type det = m[0][0] * (m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223) - m[0][1] * (m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223) + m[0][2] * (m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123) - m[0][3] * (m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123);
    return det;
}

float vsg::determinant(const mat4& m)
{
    return t_determinant<float>(m);
}

double vsg::determinant(const dmat4& m)
{
    return t_determinant<double>(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
/// decompose float matrix into translation, rotation and scale components.
template<class T>
bool t_decompose(const t_mat4<T>& m, t_vec3<T>& translation, t_quat<T>& rotation, t_vec3<T>& scale)
{
    // get the translation.
    translation = m[3].xyz;

    // compute the scale
    scale[0] = length(m[0].xyz);
    scale[1] = length(m[1].xyz);
    scale[2] = length(m[2].xyz);

    // check that we don't have any axis scaled by 0 as this would cause a
    // divide by zero in the rotation code.
    if (scale[0] == 0.0 || scale[1] == 0.0 || scale[2] == 0.0) return false;

    // compute rotation matrix and subsequently the quaternion
    t_mat3<T> rm(m[0].xyz / scale[0],
                 m[1].xyz / scale[1],
                 m[2].xyz / scale[2]);

    auto trace = rm[0][0] + rm[1][1] + rm[2][2]; // diagonal of matrix
    if (trace > static_cast<T>(0.0))
    {
        auto root = sqrt(trace + static_cast<T>(1.0));
        auto half_inv_root = static_cast<T>(0.5) / root;
        rotation.set(half_inv_root * (rm[1][2] - rm[2][1]),
                     half_inv_root * (rm[2][0] - rm[0][2]),
                     half_inv_root * (rm[0][1] - rm[1][0]),
                     static_cast<T>(0.5) * root);
    }
    else // trace <= 0.0
    {
        // locate max on diagonal
        int i = 0;
        if (rm[1][1] > rm[0][0]) i = 1;
        if (rm[2][2] > rm[i][i]) i = 2;

        // set up the orthogonal axis to the max diagonal.
        const int next[3] = {1, 2, 0};
        int j = next[i];
        int k = next[j];

        auto root = sqrt(rm[i][i] - rm[j][j] - rm[k][k] + static_cast<T>(1.0));
        auto half_inv_root = static_cast<T>(0.5) / root;
        rotation[i] = static_cast<T>(0.5) * root;
        rotation[j] = half_inv_root * (rm[i][j] + rm[j][i]);
        rotation[k] = half_inv_root * (rm[i][k] + rm[k][i]);
        rotation[3] = half_inv_root * (rm[j][k] - rm[k][j]);
    }

    return true;
}

bool vsg::decompose(const mat4& m, vec3& translation, quat& rotation, vec3& scale)
{
    return t_decompose<float>(m, translation, rotation, scale);
}

bool vsg::decompose(const dmat4& m, dvec3& translation, dquat& rotation, dvec3& scale)
{
    return t_decompose<double>(m, translation, rotation, scale);
}

bool vsg::decompose(const ldmat4& m, ldvec3& translation, ldquat& rotation, ldvec3& scale)
{
    return t_decompose<long double>(m, translation, rotation, scale);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// computeFrustumBound
//
template<typename T>
t_sphere<T> t_computeFrustumBound(const t_mat4<T>& m)
{
    using vec_type = t_vec3<T>;
    using value_type = T;
    auto inv_m = inverse(m);

    auto update_radius2 = [&](value_type& r, const vec_type& center, const vec_type& corner) -> void {
        auto new_r = length2(corner - center);
        if (new_r > r) r = new_r;
    };

    //
    // TODO : depth range should probably be 0 to 1 for Vulkan, rather than -1 to 1 for OpenGL.
    //

    // compute a2, the radius squared of the near plane relative to the near planes mid point
    vec_type near_center = inv_m * vec_type(0.0, 0.0, -1.0);
    value_type a2 = length2(inv_m * vec_type(-1.0, -1.0, -1.0) - near_center);
    update_radius2(a2, near_center, inv_m * vec_type(1.0, -1.0, -1.0));
    update_radius2(a2, near_center, inv_m * vec_type(1.0, 1.0, -1.0));
    update_radius2(a2, near_center, inv_m * vec_type(-1.0, 1.0, -1.0));

    // compute b2, the radius squared of the far plane relative to the far planes mid point
    vec_type far_center = inv_m * vec_type(0.0, 0.0, 1.0);
    value_type b2 = length2(inv_m * vec_type(-1.0, -1.0, 1.0) - far_center);
    update_radius2(b2, far_center, inv_m * vec_type(1.0, -1.0, 1.0));
    update_radius2(b2, far_center, inv_m * vec_type(1.0, 1.0, 1.0));
    update_radius2(b2, far_center, inv_m * vec_type(-1.0, 1.0, 1.0));

    // compute the position along the center line of the frustum that minimizes the radius to the near/far corners of the frustum
    value_type c2 = length2(far_center - near_center);
    value_type c = sqrt(c2);
    value_type d = (b2 + c2 - a2) / (static_cast<value_type>(2.0) * c);

    // compute radius
    value_type radius;
    if (d > c) // d beyond far plane
    {
        d = c;
        radius = sqrt(b2);
    }
    else if (d < 0.0) // d in front of near plane
    {
        d = 0.0;
        radius = sqrt(a2);
    }
    else // d between near and far planes
    {
        radius = sqrt(a2 + d * d);
    }

    auto center = near_center + (far_center - near_center) * (d / c);

    return t_sphere<T>(center, radius);
}

sphere vsg::computeFrustumBound(const mat4& m)
{
    return t_computeFrustumBound<float>(m);
}

dsphere vsg::computeFrustumBound(const dmat4& m)
{
    return t_computeFrustumBound<double>(m);
}

bool vsg::transform(CoordinateConvention source, CoordinateConvention destination, dmat4& matrix)
{
    if (source == destination || source == CoordinateConvention::NO_PREFERENCE || destination == CoordinateConvention::NO_PREFERENCE) return false;

    if (source == CoordinateConvention::X_UP)
    {
        if (destination == CoordinateConvention::Y_UP)
        {
            matrix.set(0.0, -1.0, 0.0, 0.0,
                       1.0, 0.0, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0.0, 0.0, 0.0, 1.0);
        }
        else // destination must be Z_UP
        {
            matrix.set(0.0, 0.0, 1.0, 0.0,
                       0.0, 1.0, 0.0, 0.0,
                       -1.0, 0.0, 0.0, 0.0,
                       0.0, 0.0, 0.0, 1.0);
        }
    }
    else if (source == CoordinateConvention::Y_UP)
    {
        if (destination == CoordinateConvention::X_UP)
        {
            matrix.set(0.0, 1.0, 0.0, 0.0,
                       -1.0, 0.0, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0.0, 0.0, 0.0, 1.0);
        }
        else // destination must be Z_UP
        {
            matrix.set(1.0, 0.0, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0.0, -1.0, 0.0, 0.0,
                       0.0, 0.0, 0.0, 1.0);
        }
    }
    else // source must be Z_UP
    {
        if (destination == CoordinateConvention::X_UP)
        {
            matrix.set(0.0, 0.0, -1.0, 0.0,
                       0.0, 1.0, 0.0, 0.0,
                       1.0, 0.0, 0.0, 0.0,
                       0.0, 0.0, 0.0, 1.0);
        }
        else // destination must be Y_UP
        {
            matrix.set(1.0, 0.0, 0.0, 0.0,
                       0.0, 0.0, -1.0, 0.0,
                       0.0, 1.0, 0.0, 0.0,
                       0.0, 0.0, 0.0, 1.0);
        }
    }
    return true;
}

void ComputeTransform::apply(const Transform& transform)
{
    matrix = transform.transform(matrix);
}

void ComputeTransform::apply(const MatrixTransform& mt)
{
    matrix = matrix * mt.matrix;
}

void ComputeTransform::apply(const CoordinateFrame& cf)
{
    origin = cf.origin;
    matrix = vsg::rotate(cf.rotation);
}

void ComputeTransform::apply(const Camera& camera)
{
    if (camera.viewMatrix)
    {
        matrix = matrix * camera.viewMatrix->inverse();
    }
}
