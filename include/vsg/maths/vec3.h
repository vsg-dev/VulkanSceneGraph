#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

// we can't implement the anonymous union/structs combination without causing warnings, so disabled them for just this header
#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#endif
#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#    pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

#include <vsg/core/type_name.h>

#include <cmath>

namespace vsg
{

    template<typename T>
    struct t_vec3
    {
        using value_type = T;

        union
        {
            value_type value[3];
            struct
            {
                value_type x, y, z;
            };
            struct
            {
                value_type r, g, b;
            };
            struct
            {
                value_type s, t, p;
            };
        };

        constexpr t_vec3() :
            value{} {}
        constexpr t_vec3(const t_vec3& v) :
            value{v.x, v.y, v.z} {}
        constexpr t_vec3& operator=(const t_vec3&) = default;
        constexpr t_vec3(value_type in_x, value_type in_y, value_type in_z) :
            value{in_x, in_y, in_z} {}

        template<typename R>
        constexpr explicit t_vec3(const t_vec3<R>& v) :
            value{static_cast<T>(v.x), static_cast<T>(v.y), static_cast<T>(v.z)} {}

        constexpr std::size_t size() const { return 3; }

        value_type& operator[](std::size_t i) { return value[i]; }
        value_type operator[](std::size_t i) const { return value[i]; }

        template<typename R>
        t_vec3& operator=(const t_vec3<R>& rhs)
        {
            value[0] = static_cast<value_type>(rhs[0]);
            value[1] = static_cast<value_type>(rhs[1]);
            value[2] = static_cast<value_type>(rhs[2]);
            return *this;
        }

        T* data() { return value; }
        const T* data() const { return value; }

        void set(value_type in_x, value_type in_y, value_type in_z)
        {
            x = in_x;
            y = in_y;
            z = in_z;
        }

        inline t_vec3& operator+=(const t_vec3& rhs)
        {
            value[0] += rhs.value[0];
            value[1] += rhs.value[1];
            value[2] += rhs.value[2];
            return *this;
        }

        inline t_vec3& operator-=(const t_vec3& rhs)
        {
            value[0] -= rhs.value[0];
            value[1] -= rhs.value[1];
            value[2] -= rhs.value[2];
            return *this;
        }

        inline t_vec3& operator*=(value_type rhs)
        {
            value[0] *= rhs;
            value[1] *= rhs;
            value[2] *= rhs;
            return *this;
        }

        inline t_vec3& operator/=(value_type rhs)
        {
            value_type div = 1.0 / rhs;
            value[0] *= div;
            value[1] *= div;
            value[2] *= div;
            return *this;
        }
    };

    using vec3 = t_vec3<float>;
    using dvec3 = t_vec3<double>;
    using bvec3 = t_vec3<std::int8_t>;
    using svec3 = t_vec3<std::int16_t>;
    using ivec3 = t_vec3<std::int32_t>;
    using ubvec3 = t_vec3<std::uint8_t>;
    using usvec3 = t_vec3<std::uint16_t>;
    using uivec3 = t_vec3<std::uint32_t>;

    VSG_type_name(vsg::vec3);
    VSG_type_name(vsg::dvec3);
    VSG_type_name(vsg::bvec3);
    VSG_type_name(vsg::svec3);
    VSG_type_name(vsg::ivec3);
    VSG_type_name(vsg::ubvec3);
    VSG_type_name(vsg::usvec3);
    VSG_type_name(vsg::uivec3);

    template<typename T>
    constexpr bool operator==(const t_vec3<T>& lhs, const t_vec3<T>& rhs)
    {
        return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2];
    }

    template<typename T>
    constexpr bool operator!=(const t_vec3<T>& lhs, const t_vec3<T>& rhs)
    {
        return lhs[0] != rhs[0] || lhs[1] != rhs[1] || lhs[2] != rhs[2];
    }

    template<typename T>
    constexpr bool operator<(const t_vec3<T>& lhs, const t_vec3<T>& rhs)
    {
        if (lhs[0] < rhs[0]) return true;
        if (lhs[0] > rhs[0]) return false;
        if (lhs[1] < rhs[1]) return true;
        if (lhs[1] > rhs[1]) return false;
        return lhs[2] < rhs[2];
    }

    template<typename T>
    constexpr t_vec3<T> operator-(const t_vec3<T>& lhs, const t_vec3<T>& rhs)
    {
        return t_vec3<T>(lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2]);
    }

    template<typename T>
    constexpr t_vec3<T> operator-(const t_vec3<T>& v)
    {
        return t_vec3<T>(-v[0], -v[1], -v[2]);
    }

    template<typename T>
    constexpr t_vec3<T> operator+(const t_vec3<T>& lhs, const t_vec3<T>& rhs)
    {
        return t_vec3<T>(lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2]);
    }

    template<typename T>
    constexpr t_vec3<T> operator*(const t_vec3<T>& lhs, T rhs)
    {
        return t_vec3<T>(lhs[0] * rhs, lhs[1] * rhs, lhs[2] * rhs);
    }

    template<typename T>
    constexpr t_vec3<T> operator/(const t_vec3<T>& lhs, T rhs)
    {
        T inv = static_cast<T>(1.0) / rhs;
        return t_vec3<T>(lhs[0] * inv, lhs[1] * inv, lhs[2] * inv);
    }

    template<typename T>
    constexpr T length(const t_vec3<T>& v)
    {
        return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    }

    template<typename T>
    constexpr T length2(const t_vec3<T>& v)
    {
        return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    }

    template<typename T>
    constexpr t_vec3<T> normalize(const t_vec3<T>& v)
    {
        T inverse_len = static_cast<T>(1.0) / length(v);
        return t_vec3<T>(v[0] * inverse_len, v[1] * inverse_len, v[2] * inverse_len);
    }

    template<typename T>
    constexpr T dot(const t_vec3<T>& lhs, const t_vec3<T>& rhs)
    {
        return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
    }

    template<typename T>
    constexpr t_vec3<T> cross(const t_vec3<T>& lhs, const t_vec3<T>& rhs)
    {
        return t_vec3<T>(lhs[1] * rhs[2] - rhs[1] * lhs[2],
                         lhs[2] * rhs[0] - rhs[2] * lhs[0],
                         lhs[0] * rhs[1] - rhs[0] * lhs[1]);
    }
} // namespace vsg

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
