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
    struct t_vec2
    {
        using value_type = T;

        union
        {
            value_type value[2];
            struct
            {
                value_type x, y;
            };
            struct
            {
                value_type r, g;
            };
            struct
            {
                value_type s, t;
            };
        };

        constexpr t_vec2() :
            value{} {}
        constexpr t_vec2(const t_vec2& v) :
            value{v.x, v.y} {}
        constexpr t_vec2& operator=(const t_vec2&) = default;
        constexpr t_vec2(value_type in_x, value_type in_y) :
            value{in_x, in_y} {}

        template<typename R>
        constexpr explicit t_vec2(const t_vec2<R>& v) :
            value{static_cast<T>(v.x), static_cast<T>(v.y)} {}

        constexpr std::size_t size() const { return 2; }

        value_type& operator[](std::size_t i) { return value[i]; }
        value_type operator[](std::size_t i) const { return value[i]; }

        template<typename R>
        t_vec2& operator=(const t_vec2<R>& rhs)
        {
            value[0] = static_cast<value_type>(rhs[0]);
            value[1] = static_cast<value_type>(rhs[1]);
            return *this;
        }

        T* data() { return value; }
        const T* data() const { return value; }

        void set(value_type in_x, value_type in_y)
        {
            x = in_x;
            y = in_y;
        }

        inline t_vec2& operator+=(const t_vec2& rhs)
        {
            value[0] += rhs.value[0];
            value[1] += rhs.value[1];
            return *this;
        }

        inline t_vec2& operator-=(const t_vec2& rhs)
        {
            value[0] -= rhs.value[0];
            value[1] -= rhs.value[1];
            return *this;
        }

        inline t_vec2& operator*=(value_type rhs)
        {
            value[0] *= rhs;
            value[1] *= rhs;
            return *this;
        }

        inline t_vec2& operator*=(const t_vec2& rhs)
        {
            value[0] *= rhs.value[0];
            value[1] *= rhs.value[1];
            return *this;
        }

        inline t_vec2& operator/=(value_type rhs)
        {
            if constexpr (std::is_floating_point_v<value_type>)
            {
                value_type inv = static_cast<value_type>(1.0) / rhs;
                value[0] *= inv;
                value[1] *= inv;
            }
            else
            {
                value[0] /= rhs;
                value[1] /= rhs;
            }
            return *this;
        }
    };

    using vec2 = t_vec2<float>;
    using dvec2 = t_vec2<double>;
    using bvec2 = t_vec2<std::int8_t>;
    using svec2 = t_vec2<std::int16_t>;
    using ivec2 = t_vec2<std::int32_t>;
    using ubvec2 = t_vec2<std::uint8_t>;
    using usvec2 = t_vec2<std::uint16_t>;
    using uivec2 = t_vec2<std::uint32_t>;

    VSG_type_name(vsg::vec2);
    VSG_type_name(vsg::dvec2);
    VSG_type_name(vsg::bvec2);
    VSG_type_name(vsg::svec2);
    VSG_type_name(vsg::ivec2);
    VSG_type_name(vsg::ubvec2);
    VSG_type_name(vsg::usvec2);
    VSG_type_name(vsg::uivec2);

    template<typename T>
    constexpr bool operator==(const t_vec2<T>& lhs, const t_vec2<T>& rhs)
    {
        return lhs[0] == rhs[0] && lhs[1] == rhs[1];
    }

    template<typename T>
    constexpr bool operator!=(const t_vec2<T>& lhs, const t_vec2<T>& rhs)
    {
        return lhs[0] != rhs[0] || lhs[1] != rhs[1];
    }

    template<typename T>
    constexpr bool operator<(const t_vec2<T>& lhs, const t_vec2<T>& rhs)
    {
        if (lhs[0] < rhs[0]) return true;
        if (lhs[0] > rhs[0]) return false;
        return lhs[1] < rhs[1];
    }

    template<typename T>
    constexpr t_vec2<T> operator-(const t_vec2<T>& lhs, const t_vec2<T>& rhs)
    {
        return t_vec2<T>(lhs[0] - rhs[0], lhs[1] - rhs[1]);
    }

    template<typename T>
    constexpr t_vec2<T> operator-(const t_vec2<T>& v)
    {
        return t_vec2<T>(-v[0], -v[1]);
    }

    template<typename T>
    constexpr t_vec2<T> operator+(const t_vec2<T>& lhs, const t_vec2<T>& rhs)
    {
        return t_vec2<T>(lhs[0] + rhs[0], lhs[1] + rhs[1]);
    }

    template<typename T>
    constexpr t_vec2<T> operator*(const t_vec2<T>& lhs, T rhs)
    {
        return t_vec2<T>(lhs[0] * rhs, lhs[1] * rhs);
    }

    template<typename T>
    constexpr t_vec2<T> operator*(const t_vec2<T>& lhs, const t_vec2<T>& rhs)
    {
        return t_vec2<T>(lhs[0] * rhs[0], lhs[1] * rhs[1]);
    }

    template<typename T>
    constexpr t_vec2<T> operator/(const t_vec2<T>& lhs, T rhs)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            T inv = static_cast<T>(1.0) / rhs;
            return t_vec2<T>(lhs[0] * inv, lhs[1] * inv);
        }
        else
        {
            return t_vec2<T>(lhs[0] / rhs, lhs[1] / rhs);
        }
    }

    template<typename T>
    constexpr T length(const t_vec2<T>& v)
    {
        return std::sqrt(v[0] * v[0] + v[1] * v[1]);
    }

    template<typename T>
    constexpr T length2(const t_vec2<T>& v)
    {
        return v[0] * v[0] + v[1] * v[1];
    }

    template<typename T>
    constexpr t_vec2<T> normalize(const t_vec2<T>& v)
    {
        return v / length(v);
    }

    template<typename T>
    constexpr T dot(const t_vec2<T>& lhs, const t_vec2<T>& rhs)
    {
        return lhs[0] * rhs[0] + lhs[1] * rhs[1];
    }

    /// cross product of a vec2 can be thought of cross product of vec3's with the z value of 0.0/vec3's in the xy plane.
    /// The returned value is the length of the resulting vec3 cross product, and can be treated as the signed area of the parallelogram, negative if rhs is clockwise from lhs when looking down on xy plane.
    template<typename T>
    constexpr T cross(const t_vec2<T>& lhs, const t_vec2<T>& rhs)
    {
        return (lhs[0] * rhs[1] - rhs[0] * lhs[1]);
    }

    template<typename T>
    constexpr t_vec2<T> mix(const t_vec2<T>& start, const t_vec2<T>& end, T r)
    {
        T one_minus_r = 1 - r;
        return t_vec2<T>(start[0] * one_minus_r + end[0] * r,
                         start[1] * one_minus_r + end[1] * r);
    }

} // namespace vsg

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
