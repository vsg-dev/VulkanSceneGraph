#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/plane.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>

namespace vsg
{

    template<typename T>
    struct t_mat4
    {
    public:
        using value_type = T;
        using column_type = t_vec4<T>;

        column_type value[4];

        constexpr t_mat4() :
            value{{1, 0, 0, 0},
                  {0, 1, 0, 0},
                  {0, 0, 1, 0},
                  {0, 0, 0, 1}} {}

        constexpr explicit t_mat4(value_type v) :
            value{{v, 0, 0, 0},
                  {0, v, 0, 0},
                  {0, 0, v, 0},
                  {0, 0, 0, v}} {}

        constexpr t_mat4(value_type v0, value_type v1, value_type v2, value_type v3,   /* column 0 */
                         value_type v4, value_type v5, value_type v6, value_type v7,   /* column 1 */
                         value_type v8, value_type v9, value_type v10, value_type v11, /* column 2 */
                         value_type v12, value_type v13, value_type v14, value_type v15) /* column 3 */ :
            value{{v0, v1, v2, v3},
                  {v4, v5, v6, v7},
                  {v8, v9, v10, v11},
                  {v12, v13, v14, v15}}
        {
        }

        constexpr explicit t_mat4(value_type v[16]) :
            value{{v[0], v[1], v[2], v[3]},
                  {v[4], v[5], v[6], v[7]},
                  {v[8], v[9], v[10], v[11]},
                  {v[12], v[13], v[14], v[15]}} {}

        template<typename R>
        explicit t_mat4(const t_mat4<R>& rhs)
        {
            value[0] = rhs[0];
            value[1] = rhs[1];
            value[2] = rhs[2];
            value[3] = rhs[3];
        }

        constexpr std::size_t size() const { return 16; }
        constexpr std::size_t columns() const { return 4; }
        constexpr std::size_t rows() const { return 4; }

        column_type& operator[](std::size_t c) { return value[c]; }
        const column_type& operator[](std::size_t c) const { return value[c]; }

        value_type& operator()(std::size_t c, std::size_t r) { return value[c][r]; }
        value_type operator()(std::size_t c, std::size_t r) const { return value[c][r]; }

        template<typename R>
        t_mat4& operator=(const t_mat4<R>& rhs)
        {
            value[0] = rhs[0];
            value[1] = rhs[1];
            value[2] = rhs[2];
            value[3] = rhs[3];
            return *this;
        }

        void set(value_type v0, value_type v1, value_type v2, value_type v3,     /* column 0 */
                 value_type v4, value_type v5, value_type v6, value_type v7,     /* column 1 */
                 value_type v8, value_type v9, value_type v10, value_type v11,   /* column 2 */
                 value_type v12, value_type v13, value_type v14, value_type v15) /* column 3 */
        {
            value[0].set(v0, v1, v2, v3);
            value[1].set(v4, v5, v6, v7);
            value[2].set(v8, v9, v10, v11);
            value[3].set(v12, v13, v14, v15);
        }

        T* data() { return value[0].data(); }
        const T* data() const { return value[0].data(); }
    };

    using mat4 = t_mat4<float>;
    using dmat4 = t_mat4<double>;

    VSG_type_name(vsg::mat4);
    VSG_type_name(vsg::dmat4);

    template<typename T>
    bool operator==(const t_mat4<T>& lhs, const t_mat4<T>& rhs)
    {
        return lhs.value[0] == rhs.value[0] &&
               lhs.value[1] == rhs.value[1] &&
               lhs.value[2] == rhs.value[2] &&
               lhs.value[3] == rhs.value[3];
    }

    template<typename T>
    bool operator!=(const t_mat4<T>& lhs, const t_mat4<T>& rhs)
    {
        return lhs.value[0] != rhs.value[0] ||
               lhs.value[1] != rhs.value[1] ||
               lhs.value[2] != rhs.value[2] ||
               lhs.value[3] != rhs.value[3];
    }

    template<typename T>
    bool operator<(const t_mat4<T>& lhs, const t_mat4<T>& rhs)
    {
        if (lhs.value[0] < rhs.value[0]) return true;
        if (rhs.value[0] < lhs.value[0]) return false;
        if (lhs.value[1] < rhs.value[1]) return true;
        if (rhs.value[1] < lhs.value[1]) return false;
        if (lhs.value[2] < rhs.value[2]) return true;
        if (rhs.value[2] < lhs.value[2]) return false;
        return lhs.value[3] < rhs.value[3];
    }

    template<typename T>
    T dot(const t_mat4<T>& lhs, const t_mat4<T>& rhs, int c, int r)
    {
        return lhs[0][r] * rhs[c][0] +
               lhs[1][r] * rhs[c][1] +
               lhs[2][r] * rhs[c][2] +
               lhs[3][r] * rhs[c][3];
    }

    template<typename T>
    t_mat4<T> operator*(const t_mat4<T>& lhs, const t_mat4<T>& rhs)
    {
        return t_mat4<T>(dot(lhs, rhs, 0, 0), dot(lhs, rhs, 0, 1), dot(lhs, rhs, 0, 2), dot(lhs, rhs, 0, 3),
                         dot(lhs, rhs, 1, 0), dot(lhs, rhs, 1, 1), dot(lhs, rhs, 1, 2), dot(lhs, rhs, 1, 3),
                         dot(lhs, rhs, 2, 0), dot(lhs, rhs, 2, 1), dot(lhs, rhs, 2, 2), dot(lhs, rhs, 2, 3),
                         dot(lhs, rhs, 3, 0), dot(lhs, rhs, 3, 1), dot(lhs, rhs, 3, 2), dot(lhs, rhs, 3, 3));
    }

    template<typename T>
    t_vec4<T> operator*(const t_mat4<T>& lhs, const t_vec4<T>& rhs)
    {
        return t_vec4<T>(lhs[0][0] * rhs[0] + lhs[1][0] * rhs[1] + lhs[2][0] * rhs[2] + lhs[3][0] * rhs[3],
                         lhs[0][1] * rhs[0] + lhs[1][1] * rhs[1] + lhs[2][1] * rhs[2] + lhs[3][1] * rhs[3],
                         lhs[0][2] * rhs[0] + lhs[1][2] * rhs[1] + lhs[2][2] * rhs[2] + lhs[3][2] * rhs[3],
                         lhs[0][3] * rhs[0] + lhs[1][3] * rhs[1] + lhs[2][3] * rhs[2] + lhs[3][3] * rhs[3]);
    }

    template<typename T, typename R>
    t_plane<R> operator*(const t_mat4<T>& lhs, const t_plane<R>& rhs)
    {
        t_plane<R> transformed(lhs[0][0] * rhs[0] + lhs[1][0] * rhs[1] + lhs[2][0] * rhs[2] + lhs[3][0] * rhs[3],
                               lhs[0][1] * rhs[0] + lhs[1][1] * rhs[1] + lhs[2][1] * rhs[2] + lhs[3][1] * rhs[3],
                               lhs[0][2] * rhs[0] + lhs[1][2] * rhs[1] + lhs[2][2] * rhs[2] + lhs[3][2] * rhs[3],
                               lhs[0][3] * rhs[0] + lhs[1][3] * rhs[1] + lhs[2][3] * rhs[2] + lhs[3][3] * rhs[3]);
        T inv = static_cast<R>(1.0) / length(transformed.n);
        return t_plane<T>(transformed[0] * inv, transformed[1] * inv, transformed[2] * inv, transformed[3] * inv);
    }

    template<typename T>
    t_vec4<T> operator*(const t_vec4<T>& lhs, const t_mat4<T>& rhs)
    {
        return t_vec4<T>(lhs[0] * rhs[0][0] + lhs[1] * rhs[0][1] + lhs[2] * rhs[0][2] + lhs[3] * rhs[0][3],
                         lhs[0] * rhs[1][0] + lhs[1] * rhs[1][1] + lhs[2] * rhs[1][2] + lhs[3] * rhs[1][3],
                         lhs[0] * rhs[2][0] + lhs[1] * rhs[2][1] + lhs[2] * rhs[2][2] + lhs[3] * rhs[2][3],
                         lhs[0] * rhs[3][0] + lhs[1] * rhs[3][1] + lhs[2] * rhs[3][2] + lhs[3] * rhs[3][3]);
    }

    template<typename T, typename R>
    t_plane<T> operator*(const t_plane<T>& lhs, const t_mat4<R>& rhs)
    {
        t_plane<T> transformed(lhs[0] * rhs[0][0] + lhs[1] * rhs[0][1] + lhs[2] * rhs[0][2] + lhs[3] * rhs[0][3],
                               lhs[0] * rhs[1][0] + lhs[1] * rhs[1][1] + lhs[2] * rhs[1][2] + lhs[3] * rhs[1][3],
                               lhs[0] * rhs[2][0] + lhs[1] * rhs[2][1] + lhs[2] * rhs[2][2] + lhs[3] * rhs[2][3],
                               lhs[0] * rhs[3][0] + lhs[1] * rhs[3][1] + lhs[2] * rhs[3][2] + lhs[3] * rhs[3][3]);
        T inv = static_cast<T>(1.0) / length(transformed.n);
        return t_plane<T>(transformed[0] * inv, transformed[1] * inv, transformed[2] * inv, transformed[3] * inv);
    }

    template<typename T>
    t_vec3<T> operator*(const t_mat4<T>& lhs, const t_vec3<T>& rhs)
    {
        T inv = static_cast<T>(1.0) / (lhs[0][3] * rhs[0] + lhs[1][3] * rhs[1] + lhs[2][3] * rhs[2] + lhs[3][3]);
        return t_vec3<T>((lhs[0][0] * rhs[0] + lhs[1][0] * rhs[1] + lhs[2][0] * rhs[2] + lhs[3][0]) * inv,
                         (lhs[0][1] * rhs[0] + lhs[1][1] * rhs[1] + lhs[2][1] * rhs[2] + lhs[3][1]) * inv,
                         (lhs[0][2] * rhs[0] + lhs[1][2] * rhs[1] + lhs[2][2] * rhs[2] + lhs[3][2]) * inv);
    }

    template<typename T>
    t_vec3<T> operator*(const t_vec3<T>& lhs, const t_mat4<T>& rhs)
    {
        T inv = static_cast<T>(1.0) / (lhs[0] * rhs[3][0] + lhs[1] * rhs[3][1] + lhs[2] * rhs[3][2] + rhs[3][3]);
        return t_vec3<T>(lhs[0] * rhs[0][0] + lhs[1] * rhs[0][1] + lhs[2] * rhs[0][2] + rhs[0][3] * inv,
                         lhs[0] * rhs[1][0] + lhs[1] * rhs[1][1] + lhs[2] * rhs[1][2] + rhs[1][3] * inv,
                         lhs[0] * rhs[2][0] + lhs[1] * rhs[2][1] + lhs[2] * rhs[2][2] + rhs[2][3] * inv);
    }

} // namespace vsg
