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

#include <vsg/maths/vec3.h>

namespace vsg
{

    /// t_vec4 template class that a represents a 4D vector
    template<typename T>
    struct t_vec4
    {
        using value_type = T;

        union
        {
            value_type value[4];
            struct
            {
                value_type x, y, z, w;
            };
            struct
            {
                value_type r, g, b, a;
            };
            struct
            {
                value_type s, t, p, q;
            };
            t_vec3<T> xyz;
        };

        constexpr t_vec4() :
            value{} {}
        constexpr t_vec4(const t_vec4& v) :
            value{v.x, v.y, v.z, v.w} {}
        constexpr t_vec4& operator=(const t_vec4&) = default;

        constexpr t_vec4(value_type in_x, value_type in_y, value_type in_z, value_type in_w) :
            value{in_x, in_y, in_z, in_w} {}

        template<typename R>
        constexpr explicit t_vec4(const t_vec4<R>& v) :
            value{static_cast<T>(v.x), static_cast<T>(v.y), static_cast<T>(v.z), static_cast<T>(v.w)} {}

        template<typename R>
        constexpr t_vec4(const t_vec2<R>& v, value_type in_z, value_type in_w) :
            value{static_cast<T>(v.x), static_cast<T>(v.y), in_z, in_w} {}

        template<typename R>
        constexpr t_vec4(const t_vec3<R>& v, value_type in_w) :
            value{static_cast<T>(v.x), static_cast<T>(v.y), static_cast<T>(v.z), in_w} {}

        constexpr std::size_t size() const { return 4; }

        value_type& operator[](std::size_t i) { return value[i]; }
        value_type operator[](std::size_t i) const { return value[i]; }

        template<typename R>
        t_vec4& operator=(const t_vec4<R>& rhs)
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

        inline t_vec4& operator+=(const t_vec4& rhs)
        {
            value[0] += rhs.value[0];
            value[1] += rhs.value[1];
            value[2] += rhs.value[2];
            value[3] += rhs.value[3];
            return *this;
        }

        inline t_vec4& operator-=(const t_vec4& rhs)
        {
            value[0] -= rhs.value[0];
            value[1] -= rhs.value[1];
            value[2] -= rhs.value[2];
            value[3] -= rhs.value[3];
            return *this;
        }

        inline t_vec4& operator*=(value_type rhs)
        {
            value[0] *= rhs;
            value[1] *= rhs;
            value[2] *= rhs;
            value[3] *= rhs;
            return *this;
        }

        inline t_vec4& operator*=(const t_vec4& rhs)
        {
            value[0] *= rhs.value[0];
            value[1] *= rhs.value[1];
            value[2] *= rhs.value[2];
            value[3] *= rhs.value[3];
            return *this;
        }

        inline t_vec4& operator/=(value_type rhs)
        {
            if constexpr (std::is_floating_point_v<value_type>)
            {
                value_type inv = static_cast<value_type>(1.0) / rhs;
                value[0] *= inv;
                value[1] *= inv;
                value[2] *= inv;
                value[3] *= inv;
            }
            else
            {
                value[0] /= rhs;
                value[1] /= rhs;
                value[2] /= rhs;
                value[3] /= rhs;
            }
            return *this;
        }
    };

    using vec4 = t_vec4<float>;           // float 4D vector
    using dvec4 = t_vec4<double>;         // double 4D vector
    using bvec4 = t_vec4<std::int8_t>;    // signed 8 bit integer 4D vector
    using svec4 = t_vec4<std::int16_t>;   //  signed 16 bit integer 4D vector
    using ivec4 = t_vec4<std::int32_t>;   //  signed 32 bit integer 4D vector
    using ubvec4 = t_vec4<std::uint8_t>;  //  unsigned 8 bit integer 4D vector
    using usvec4 = t_vec4<std::uint16_t>; //  unsigned 16 bit integer 4D vector
    using uivec4 = t_vec4<std::uint32_t>; //  unsigned 32 bit integer 4D vector

    VSG_type_name(vsg::vec4);
    VSG_type_name(vsg::dvec4);
    VSG_type_name(vsg::bvec4);
    VSG_type_name(vsg::svec4);
    VSG_type_name(vsg::ivec4);
    VSG_type_name(vsg::ubvec4);
    VSG_type_name(vsg::usvec4);
    VSG_type_name(vsg::uivec4);

    template<typename T>
    constexpr bool operator==(const t_vec4<T>& lhs, const t_vec4<T>& rhs)
    {
        return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
    }

    template<typename T>
    constexpr bool operator!=(const t_vec4<T>& lhs, const t_vec4<T>& rhs)
    {
        return lhs[0] != rhs[0] || lhs[1] != rhs[1] || lhs[2] != rhs[2] || lhs[3] != rhs[3];
    }

    template<typename T>
    constexpr bool operator<(const t_vec4<T>& lhs, const t_vec4<T>& rhs)
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
    constexpr t_vec4<T> operator-(const t_vec4<T>& lhs, const t_vec4<T>& rhs)
    {
        return t_vec4<T>(lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2], lhs[3] - rhs[3]);
    }

    template<typename T>
    constexpr t_vec4<T> operator-(const t_vec4<T>& v)
    {
        return t_vec4<T>(-v[0], -v[1], -v[2], -v[3]);
    }

    template<typename T>
    constexpr t_vec4<T> operator+(const t_vec4<T>& lhs, const t_vec4<T>& rhs)
    {
        return t_vec4<T>(lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2], lhs[3] + rhs[3]);
    }

    template<typename T>
    constexpr t_vec4<T> operator*(const t_vec4<T>& lhs, T rhs)
    {
        return t_vec4<T>(lhs[0] * rhs, lhs[1] * rhs, lhs[2] * rhs, lhs[3] * rhs);
    }

    template<typename T>
    constexpr t_vec4<T> operator*(const t_vec4<T>& lhs, const t_vec4<T>& rhs)
    {
        return t_vec4<T>(lhs[0] * rhs[0], lhs[1] * rhs[1], lhs[2] * rhs[2], lhs[3] * rhs[3]);
    }

    template<typename T>
    constexpr t_vec4<T> operator/(const t_vec4<T>& lhs, T rhs)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            T inv = static_cast<T>(1.0) / rhs;
            return t_vec4<T>(lhs[0] * inv, lhs[1] * inv, lhs[2] * inv, lhs[3] * inv);
        }
        else
        {
            return t_vec4<T>(lhs[0] / rhs, lhs[1] / rhs, lhs[2] / rhs, lhs[3] / rhs);
        }
    }

    template<typename T>
    constexpr T length(const t_vec4<T>& v)
    {
        return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
    }

    template<typename T>
    constexpr T length2(const t_vec4<T>& v)
    {
        return v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];
    }

    template<typename T>
    constexpr t_vec4<T> normalize(const t_vec4<T>& v)
    {
        return v / length(v);
    }

    template<typename T>
    constexpr t_vec4<T> mix(const t_vec4<T>& start, const t_vec4<T>& end, T r)
    {
        T one_minus_r = 1 - r;
        return t_vec4<T>(start[0] * one_minus_r + end[0] * r,
                         start[1] * one_minus_r + end[1] * r,
                         start[2] * one_minus_r + end[2] * r,
                         start[3] * one_minus_r + end[3] * r);
    }

} // namespace vsg

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
