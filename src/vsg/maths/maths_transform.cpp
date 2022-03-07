/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/viewer/Camera.h>

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
// compute determinate of a matrix
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

float determinant(const mat4& m)
{
    return t_determinant<float>(m);
}

double determinant(const dmat4& m)
{
    return t_determinant<double>(m);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
/// decompose float matrix into scale, orientation, position, skew and pespective components.
//
template<class T>
bool t_decompose(const t_mat4<T>& m, t_vec3<T>& scale, t_quat<T>& orientation, t_vec3<T>& translation, t_vec3<T>& skew, t_vec4<T>& perspective)
{
    // implementation inspired by glm::decompose(..) and GraphicsGemsIII TransformMatrix.cpp decompose(..) function

    // TODO use epsilon test in comparisons.

    // normalize matrix
    if (m[0][0] == static_cast<T>(0.0)) return false;

    t_mat4<T> localMatrix = m;

    T div = static_cast<T>(1.0) / m[3][3];
    localMatrix[0] *= div;
    localMatrix[1] *= div;
    localMatrix[2] *= div;
    localMatrix[3] *= div;

    // isolate perspective matrix
    if (localMatrix[0][3] != static_cast<T>(0.0) || localMatrix[1][3] != static_cast<T>(0.0) || localMatrix[2][3] != static_cast<T>(0.0))
    {
        t_mat4<T> perspectiveMatrix = localMatrix;

        for(int i = 0; i < 3; ++i)
            perspectiveMatrix[i][3] = static_cast<T>(0.0);
        perspectiveMatrix[3][3] = static_cast<T>(1.0);

        if (t_determinant<T>(perspectiveMatrix) == static_cast<T>(0.0))
            return false;

        // rightHandSide is the right hand side of the equation
        t_vec4<T> rightHandSide(localMatrix[0][3], localMatrix[1][3], localMatrix[2][3], localMatrix[3][3]);
        t_mat4<T> transposedInversePerspectiveMatrix = transpose(inverse(perspectiveMatrix));
        perspective = transposedInversePerspectiveMatrix * rightHandSide;

        // clear the perspective portion;
        localMatrix[0][3] = localMatrix[1][3] = localMatrix[2][3] = static_cast<T>(0.0);
        localMatrix[3][3] = static_cast<T>(1.0);
    }
    else
    {
        perspective.set(static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(1.0));
    }

    // translation
    translation.set(localMatrix[3][0], localMatrix[3][1], localMatrix[3][2]);
    localMatrix[3][0] = localMatrix[3][1] = localMatrix[3][2] = static_cast<T>(0.0);

    // scale and shear
    t_vec3<T> row[3];
    for(size_t i = 0; i < 3; ++i)
        for(size_t j = 0; j < 3; ++j)
            row[i][j] = localMatrix[i][j];

    scale.x = length(row[0]);

    // row[0] = scale (row[0], 1.0) ??

    // compute xy shared factor and make 2nd row orthogonal to 1st.
    skew.z = dot(row[0], row[1]);
    row[1] -= row[0] * skew.z;

    // Now compute Y scale and normalize 2nd row.
    scale.y = length(row[1]);
    row[1] = normalize(row[1]);
    skew.z /= scale.y;

    // compute xz and yz shreads, othorogonalize 3rd row.
    skew.y = dot(row[0], row[1]);
    row[2] -= row[0] * skew.y;
    skew.x = dot(row[1], row[2]);
    row[2] -= row[1] * skew.x;

    // next get z scale and normalize 3rd row
    scale.z = length(row[2]);
    row[2] = normalize(row[2]);
    skew.x /= scale.z;
    skew.y /= scale.z;

    // matrix in rows[] os now orthogonal
    // check for coordinate syste fli, if the determinate is -1 then negate the matrix and scaling factors.
    auto Pdum3 = cross(row[1], row[2]);
    if (dot(row[0], Pdum3) < static_cast<T>(0.0))
    {
        for(size_t i = 0; i < 3; ++i)
        {
            scale[i] *= static_cast<T>(-1.0);
            row[i] *= static_cast<T>(-1.0);
        }
    }

    // get rotation
    auto trace = row[0].x + row[1].y + row[2].z; // diagonal of row[] matrix
    if (trace > static_cast<T>(0.0))
    {
        auto root = sqrt(trace + static_cast<T>(1.0));
        auto half_inv_root = static_cast<T>(0.5) / root;
        orientation.set(half_inv_root * (row[1].z - row[2].y),
                        half_inv_root * (row[2].x - row[0].z),
                        half_inv_root * (row[0].y - row[1].x),
                        static_cast<T>(0.5) * root);

    }
    else // trace <= 0.0
    {
        // locate max on diagonal
        int i = 0;
        if (row[1].y > row[0][0]) i = 1;
        if (row[2].z > row[i][i]) i = 2;

        // set up the othrogonal axis to the max diagonal.
        int next[3] = {1, 2, 0};
        int j = next[i];
        int k = next[j];

        auto root = sqrt(row[i][i] - row[j][j] - row[k][k] + static_cast<T>(1.0));
        auto half_inv_root = static_cast<T>(0.5) / root;
        orientation[i] = static_cast<T>(0.5) / root;
        orientation[j] = half_inv_root * (row[i][j] + row[j][i]);
        orientation[k] = half_inv_root * (row[i][k] + row[k][i]);
        orientation[3] = half_inv_root * (row[j][k] - row[k][j]);
    }

    return true;
}

bool vsg::decompose(const mat4& m, vec3& scale, quat& orientation, vec3& translation, vec3& skew, vec4& perspective)
{
    return t_decompose<float>(m, scale, orientation, translation, skew, perspective);
}

bool vsg::decompose(const dmat4& m, dvec3& scale, dquat& orientation, dvec3& translation, dvec3& skew, dvec4& perspective)
{
    return t_decompose<double>(m, scale, orientation, translation, skew, perspective);
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

    // compute the a2 the radius squared of the near plane relative to the near planes mid point
    vec_type near_center = inv_m * vec_type(0.0, 0.0, -1.0);
    value_type a2 = length2(inv_m * vec_type(-1.0, -1.0, -1.0) - near_center);
    update_radius2(a2, near_center, inv_m * vec_type(1.0, -1.0, -1.0));
    update_radius2(a2, near_center, inv_m * vec_type(1.0, 1.0, -1.0));
    update_radius2(a2, near_center, inv_m * vec_type(-1.0, 1.0, -1.0));

    // compute the b2 the radius squared of the far plane relative to the far planes mid point
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
            matrix.set(0.0, 1.0, 0.0, 0.0,
                       -1.0, 0.0, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0.0, 0.0, 0.0, 1.0);
        }
        else // destination most be Z_UP
        {
            matrix.set(0.0, 0.0, 1.0, 0.0,
                       -1.0, 0.0, 0.0, 0.0,
                       0.0, -1.0, 0.0, 0.0,
                       0.0, 0.0, 0.0, 1.0);
        }
    }
    else if (source == CoordinateConvention::Y_UP)
    {
        if (destination == CoordinateConvention::X_UP)
        {
            matrix.set(0.0, -1.0, 0.0, 0.0,
                       1.0, 0.0, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0.0, 0.0, 0.0, 1.0);
        }
        else // destination most be Z_UP
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
            matrix.set(0.0, -1.0, 0.0, 0.0,
                       1.0, 0.0, -1.0, 0.0,
                       0.0, 0.0, -0.0, 0.0,
                       0.0, 0.0, 0.0, 1.0);
        }
        else // destination most be Y_UP
        {
            matrix.set(0.0, 0.0, 1.0, 0.0,
                       -1.0, 0.0, 0.0, 0.0,
                       0.0, -1.0, 0.0, 0.0,
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

void ComputeTransform::apply(const Camera& camera)
{
    if (camera.viewMatrix)
    {
        matrix = matrix * camera.viewMatrix->inverse();
    }
}
