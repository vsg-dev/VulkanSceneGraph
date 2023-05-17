#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

// we can't implement the anonymous union/structs combination without causing warnings, so disabled them for just this header

#include <vsg/maths/mat4.h>

#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#endif
#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#    pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

namespace vsg
{

    /// t_quat template class that a represents quaternion
    template<typename T>
    struct t_quat
    {
        using value_type = T;

        union
        {
            value_type value[4];
            struct
            {
                value_type x, y, z, w;
            };
        };

        constexpr t_quat() :
            value{} {}
        constexpr t_quat(const t_quat& v) :
            value{v.x, v.y, v.z, v.w} {}
        constexpr t_quat(value_type in_x, value_type in_y, value_type in_z, value_type in_w) :
            value{in_x, in_y, in_z, in_w} {}
        constexpr t_quat(value_type angle_radians, const t_vec3<value_type>& axis)
        {
            set(angle_radians, axis);
        }
        constexpr t_quat(const t_vec3<value_type>& from, const t_vec3<value_type>& to)
        {
            set(from, to);
        }

        template<typename R>
        constexpr explicit t_quat(const t_quat<R>& v) :
            value{static_cast<T>(v.x), static_cast<T>(v.y), static_cast<T>(v.z), static_cast<T>(v.w)} {}

        constexpr t_quat& operator=(const t_quat&) = default;

        constexpr std::size_t size() const { return 4; }

        value_type& operator[](std::size_t i) { return value[i]; }
        value_type operator[](std::size_t i) const { return value[i]; }

        template<typename R>
        t_quat& operator=(const t_quat<R>& rhs)
        {
            value[0] = static_cast<value_type>(rhs[0]);
            value[1] = static_cast<value_type>(rhs[1]);
            value[2] = static_cast<value_type>(rhs[2]);
            value[3] = static_cast<value_type>(rhs[3]);
            return *this;
        }

        T* data() { return value; }
        const T* data() const { return value; }

        void set(value_type in_x, value_type in_y, value_type in_z, value_type in_w)
        {
            x = in_x;
            y = in_y;
            z = in_z;
            w = in_w;
        }

        void set(value_type angle_radians, const t_vec3<value_type>& axis)
        {
            const value_type epsilon = 1e-7;
            value_type len = length(axis);
            if (len < epsilon)
            {
                // ~zero length axis, so reset rotation to zero.
                *this = {};
                return;
            }

            value_type inversenorm = 1.0 / len;
            value_type coshalfangle = cos(0.5 * angle_radians);
            value_type sinhalfangle = sin(0.5 * angle_radians);

            x = axis.x * sinhalfangle * inversenorm;
            y = axis.y * sinhalfangle * inversenorm;
            z = axis.z * sinhalfangle * inversenorm;
            w = coshalfangle;
        }

        void set(const t_vec3<value_type>& from, const t_vec3<value_type>& to)
        {
            const value_type epsilon = 1e-7;

            value_type dot_pd = vsg::dot(from, to);
            value_type div = std::sqrt(length2(from) * length2(to));
            vsg::dvec3 axis;
            if (div - dot_pd < epsilon)
            {
                axis = orthogonal(from);
            }
            else
            {
                axis = cross(from, to);
            }

            value_type len = length(axis);

            double angle_radians = acos(dot_pd / div);

            value_type inversenorm = 1.0 / len;
            value_type coshalfangle = cos(0.5 * angle_radians);
            value_type sinhalfangle = sin(0.5 * angle_radians);

            x = axis.x * sinhalfangle * inversenorm;
            y = axis.y * sinhalfangle * inversenorm;
            z = axis.z * sinhalfangle * inversenorm;
            w = coshalfangle;
        }

        explicit operator bool() const noexcept { return value[0] != 0.0 || value[1] != 0.0 || value[2] != 0.0 || value[3] != 0.0; }
    };

    using quat = t_quat<float>;   /// float quaternion
    using dquat = t_quat<double>; /// double quaternion

    VSG_type_name(vsg::quat);
    VSG_type_name(vsg::dquat);

    template<typename T>
    constexpr bool operator==(const t_quat<T>& lhs, const t_quat<T>& rhs)
    {
        return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
    }

    template<typename T>
    constexpr bool operator!=(const t_quat<T>& lhs, const t_quat<T>& rhs)
    {
        return lhs[0] != rhs[0] || lhs[1] != rhs[1] || lhs[2] != rhs[2] || lhs[3] != rhs[3];
    }

    template<typename T>
    constexpr bool operator<(const t_quat<T>& lhs, const t_quat<T>& rhs)
    {
        if (lhs[0] < rhs[0]) return true;
        if (lhs[0] > rhs[0]) return false;
        if (lhs[1] < rhs[1]) return true;
        if (lhs[1] > rhs[1]) return false;
        if (lhs[2] < rhs[2]) return true;
        if (lhs[2] > rhs[2]) return false;
        return lhs[3] < rhs[3];
    }

    template<typename T>
    constexpr t_quat<T> operator-(const t_quat<T>& lhs, const t_quat<T>& rhs)
    {
        return t_quat<T>(lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2], lhs[3] - rhs[3]);
    }

    template<typename T>
    constexpr t_quat<T> conjugate(const t_quat<T>& v)
    {
        return t_quat<T>(-v[0], -v[1], -v[2], v[3]);
    }

    template<typename T>
    constexpr t_quat<T> operator-(const t_quat<T>& v)
    {
        return conjugate(v);
    }

    template<typename T>
    constexpr t_quat<T> operator+(const t_quat<T>& lhs, const t_quat<T>& rhs)
    {
        return t_quat<T>(lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2], lhs[3] + rhs[3]);
    }

    // Rotate a quaternion by another quaternion
    template<typename T>
    constexpr t_quat<T> operator*(const t_quat<T>& lhs, const t_quat<T>& rhs)
    {
        t_quat<T> q(rhs[3] * lhs[0] + rhs[0] * lhs[3] + rhs[1] * lhs[2] - rhs[2] * lhs[1],
                    rhs[3] * lhs[1] - rhs[0] * lhs[2] + rhs[1] * lhs[3] + rhs[2] * lhs[0],
                    rhs[3] * lhs[2] + rhs[0] * lhs[1] - rhs[1] * lhs[0] + rhs[2] * lhs[3],
                    rhs[3] * lhs[3] - rhs[0] * lhs[0] - rhs[1] * lhs[1] - rhs[2] * lhs[2]);

        return q;
    }

    // Rotate a vector by a quaternion
    template<typename T>
    constexpr t_vec3<T> operator*(const t_quat<T>& q, const t_vec3<T>& v)
    {
        // nVidia SDK implementation
        t_vec3<T> uv, uuv;
        t_vec3<T> qvec(q[0], q[1], q[2]);
        uv = cross(qvec, v);
        uuv = cross(qvec, uv);
        T two(2.0);
        uv *= (two * q[3]);
        uuv *= two;
        return v + uv + uuv;
    }

    template<typename T>
    constexpr t_quat<T> operator*(const t_quat<T>& lhs, T rhs)
    {
        return t_quat<T>(lhs[0] * rhs, lhs[1] * rhs, lhs[2] * rhs, lhs[3] * rhs);
    }

    template<typename T>
    constexpr t_quat<T> operator/(const t_quat<T>& lhs, T rhs)
    {
        T inv = static_cast<T>(1.0) / rhs;
        return t_quat<T>(lhs[0] * inv, lhs[1] * inv, lhs[2] * inv, lhs[3] * inv);
    }

    template<typename T>
    constexpr T length(const t_quat<T>& v)
    {
        return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
    }

    template<typename T>
    constexpr t_quat<T> normalize(const t_quat<T>& v)
    {
        T inverse_len = static_cast<T>(1.0) / length(v);
        return t_quat<T>(v[0] * inverse_len, v[1] * inverse_len, v[2] * inverse_len, v[3] * inverse_len);
    }

    template<typename T>
    constexpr T dot(const t_quat<T>& lhs, const t_quat<T>& rhs)
    {
        return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2] + lhs[3] * rhs[3];
    }

    template<typename T>
    constexpr t_quat<T> inverse(const t_quat<T>& v)
    {
        t_quat<T> c = conjugate(v);
        T inverse_len = static_cast<T>(1.0) / length(v);
        return t_quat<T>(c[0] * inverse_len, c[1] * inverse_len, c[2] * inverse_len, c[3] * inverse_len);
    }

    template<typename T>
    constexpr t_quat<T> mix(const t_quat<T>& from, t_quat<T> to, T r)
    {
        T epsilon = std::numeric_limits<T>::epsilon();
        T one(1.0);

        T cosomega = dot(from, to);
        if (cosomega < 0.0)
        {
            cosomega = -cosomega;
            to.x = -to.x;
            to.y = -to.y;
            to.z = -to.z;
            to.w = -to.w;
        }

        if ((one - cosomega) > epsilon)
        {
            T omega = acos(cosomega);
            T sinomega = sin(omega);
            T scale_from = sin((one - r) * omega) / sinomega;
            T scale_to = sin(r * omega) / sinomega;
            return (from * scale_from) + (to * scale_to);
        }
        else
        {
            // quaternion's are very close so just linearly interpolate
            return (from * (one - r)) + (to * r);
        }
    }

} // namespace vsg

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
