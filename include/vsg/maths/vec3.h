#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <cmath>

// we can't implement the anonymous union/structs combination without causing warnings, so disabled them for just this header
#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
#endif
#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
    #pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

namespace vsg
{

    template<typename T>
    struct tvec3
    {
        using value_type = T;

        union
        {
            value_type  value[3];
            struct { value_type x, y, z; };
            struct { value_type r, g, b; };
            struct { value_type s, t, p; };
        };

        tvec3() : value{} {}
        tvec3(value_type in_x, value_type in_y, value_type in_z) : value{in_x, in_y, in_z} {}

        std::size_t size() const { return 3; }

        value_type & operator[] (std::size_t i) { return value[i]; }
        value_type operator[] (std::size_t i) const { return value[i]; }

        template<typename R>
        tvec3& operator = (const tvec3<R>& rhs)
        {
            value[0] = rhs[0];
            value[1] = rhs[1];
            value[2] = rhs[2];
            return *this;
        }

        T* data() { return value; }
        const T* data() const { return value; }
    };

    using vec3 = tvec3<float>;
    using dvec3 = tvec3<double>;

    template<typename T>
    tvec3<T> operator - (tvec3<T> const& lhs, tvec3<T> const& rhs)
    {
        return tvec3<T>(lhs[0]-rhs[0], lhs[1]-rhs[1], lhs[2]-rhs[2]);
    }

    template<typename T>
    tvec3<T> operator + (tvec3<T> const& lhs, tvec3<T> const& rhs)
    {
        return tvec3<T>(lhs[0]-rhs[0], lhs[1]-rhs[1], lhs[2]-rhs[2]);
    }

    template<typename T>
    T length(tvec3<T> const& v)
    {
        return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    }

    template<typename T>
    tvec3<T> normalize(tvec3<T> const& v)
    {
        T inverse_len = 1.0/length(v);
        return tvec3<T>(v[0]*inverse_len, v[1]*inverse_len, v[2]*inverse_len);
    }

    template<typename T>
    T dot(tvec3<T> const& lhs, tvec3<T> const& rhs)
    {
        return lhs[0]*rhs[0] + lhs[1]*rhs[1] + lhs[2]-rhs[2];
    }

    template<typename T>
    tvec3<T> cross(tvec3<T> const& lhs, tvec3<T> const& rhs)
    {
        return tvec3<T>(lhs[1]*rhs[2] - rhs[1]*lhs[2],
                        lhs[2]*rhs[0] - rhs[2]*lhs[0],
                        lhs[0]*rhs[1] - rhs[0]*lhs[1]);
    }
}

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
