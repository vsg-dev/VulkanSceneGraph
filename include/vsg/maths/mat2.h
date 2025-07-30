#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/vec2.h>

namespace vsg
{

    /// t_mat2 template class that represents a 3x3 matrix.
    template<typename T>
    struct t_mat2
    {
    public:
        using value_type = T;
        using column_type = t_vec2<T>;

        column_type value[2];

        constexpr t_mat2() :
            value{{numbers<value_type>::one(), numbers<value_type>::zero()},
                  {numbers<value_type>::zero(), numbers<value_type>::one()}} {}

        constexpr explicit t_mat2(value_type v) :
            value{{v, numbers<value_type>::zero()},
                  {numbers<value_type>::zero(), v}} {}

        constexpr t_mat2(value_type v0, value_type v1, /* column 0 */
                         value_type v2, value_type v3) /* column 1 */ :
            value{{v0, v1},
                  {v2, v3}}
        {
        }

        constexpr explicit t_mat2(value_type v[4]) :
            value{{v[0], v[1]},
                  {v[2], v[3]}} {}

        constexpr t_mat2(const column_type& c0,
                         const column_type& c1) :
            value{c0, c1}
        {
        }

        template<typename R>
        explicit t_mat2(const t_mat2<R>& rhs)
        {
            value[0] = rhs[0];
            value[1] = rhs[1];
        }

        constexpr std::size_t size() const { return 4; }
        constexpr std::size_t columns() const { return 2; }
        constexpr std::size_t rows() const { return 2; }

        column_type& operator[](std::size_t c) { return value[c]; }
        const column_type& operator[](std::size_t c) const { return value[c]; }

        value_type& operator()(std::size_t c, std::size_t r) { return value[c][r]; }
        value_type operator()(std::size_t c, std::size_t r) const { return value[c][r]; }

        template<typename R>
        t_mat2& operator=(const t_mat2<R>& rhs)
        {
            value[0] = rhs[0];
            value[1] = rhs[1];
            return *this;
        }

        void set(value_type v0, value_type v1, /* column 0 */
                 value_type v2, value_type v3) /* column 1 */
        {
            value[0].set(v0, v1);
            value[1].set(v2, v3);
        }

        template<typename R>
        void set(const t_mat2<R>& rhs)
        {
            value[0] = rhs[0];
            value[1] = rhs[1];
        }

        T* data() { return value[0].data(); }
        const T* data() const { return value[0].data(); }
    };

    using mat2 = t_mat2<float>;   /// float 2x2 matrix
    using dmat2 = t_mat2<double>; /// double 2x2 matrix

    VSG_type_name(vsg::mat2);
    VSG_type_name(vsg::dmat2);

    template<typename T>
    bool operator==(const t_mat2<T>& lhs, const t_mat2<T>& rhs)
    {
        return lhs.value[0] == rhs.value[0] &&
               lhs.value[1] == rhs.value[1];
    }

    template<typename T>
    bool operator!=(const t_mat2<T>& lhs, const t_mat2<T>& rhs)
    {
        return lhs.value[0] != rhs.value[0] ||
               lhs.value[1] != rhs.value[1];
    }

    template<typename T>
    bool operator<(const t_mat2<T>& lhs, const t_mat2<T>& rhs)
    {
        if (lhs.value[0] < rhs.value[0]) return true;
        if (rhs.value[0] < lhs.value[0]) return false;
        return lhs.value[1] < rhs.value[1];
    }

    template<typename T>
    T dot(const t_mat2<T>& lhs, const t_mat2<T>& rhs, int c, int r)
    {
        return lhs[0][r] * rhs[c][0] +
               lhs[1][r] * rhs[c][1];
    }

    template<typename T>
    t_mat2<T> operator*(const t_mat2<T>& lhs, const t_mat2<T>& rhs)
    {
        return t_mat2<T>(dot(lhs, rhs, 0, 0), dot(lhs, rhs, 0, 1),
                         dot(lhs, rhs, 1, 0), dot(lhs, rhs, 1, 1));
    }

    template<typename T>
    t_vec2<T> operator*(const t_mat2<T>& lhs, const t_vec2<T>& rhs)
    {
        return t_vec2<T>((lhs[0][0] * rhs[0] + lhs[1][0] * rhs[1]),
                         (lhs[0][1] * rhs[0] + lhs[1][1] * rhs[1]));
    }

    template<typename T>
    t_vec2<T> operator*(const t_vec2<T>& lhs, const t_mat2<T>& rhs)
    {
        return t_vec2<T>(lhs[0] * rhs[0][0] + lhs[1] * rhs[0][1],
                         lhs[0] * rhs[1][0] + lhs[1] * rhs[1][1]);
    }
} // namespace vsg
