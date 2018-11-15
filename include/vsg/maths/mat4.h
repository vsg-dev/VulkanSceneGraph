#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

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

        constexpr t_mat4(value_type v0, value_type v1, value_type v2, value_type v3,
                         value_type v4, value_type v5, value_type v6, value_type v7,
                         value_type v8, value_type v9, value_type v10, value_type v11,
                         value_type v12, value_type v13, value_type v14, value_type v15) :
            value{{v0, v4, v8, v12},
                  {v1, v5, v9, v13},
                  {v2, v6, v10, v14},
                  {v3, v7, v11, v15}} {}

        constexpr explicit t_mat4(value_type v[16]) :
            value{{v[0], v[4], v[8], v[12]},
                  {v[1], v[5], v[9], v[13]},
                  {v[2], v[6], v[10], v[14]},
                  {v[3], v[7], v[11], v[15]}} {}

        template<typename R>
        t_mat4(const t_mat4<R>& rhs)
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
        column_type const& operator[](std::size_t c) const { return value[c]; }

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

        T* data() { return value[0].data(); }
        const T* data() const { return value[0].data(); }
    };

    using mat4 = t_mat4<float>;
    using dmat4 = t_mat4<double>;

    VSG_type_name(vsg::mat4)
    VSG_type_name(vsg::dmat4)

    template<typename T>
    T dot(const t_mat4<T>& lhs, const t_mat4<T>& rhs, int c, int r)
    {
        return lhs[0][r] * rhs[c][0] +
               lhs[1][r] * rhs[c][1] +
               lhs[2][r] * rhs[c][2] +
               lhs[3][r] * rhs[c][3];
    }

    template<typename T>
    t_mat4<T> operator*(t_mat4<T> const& lhs, t_mat4<T> const& rhs)
    {
        return t_mat4<T>(dot(lhs, rhs, 0, 0), dot(lhs, rhs, 1, 0), dot(lhs, rhs, 2, 0), dot(lhs, rhs, 3, 0),
                         dot(lhs, rhs, 0, 1), dot(lhs, rhs, 1, 1), dot(lhs, rhs, 2, 1), dot(lhs, rhs, 3, 1),
                         dot(lhs, rhs, 0, 2), dot(lhs, rhs, 1, 2), dot(lhs, rhs, 2, 2), dot(lhs, rhs, 3, 2),
                         dot(lhs, rhs, 0, 3), dot(lhs, rhs, 1, 3), dot(lhs, rhs, 2, 3), dot(lhs, rhs, 3, 3));
    }

} // namespace vsg
