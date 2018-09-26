#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

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
    struct t_vec2
    {
        using value_type = T;

        union
        {
            value_type  value[2];
            struct { value_type x, y; };
            struct { value_type r, g; };
            struct { value_type s, t; };
        };

        constexpr t_vec2() : value{} {}
        constexpr t_vec2(value_type in_x, value_type in_y) : value{in_x, in_y} {}

        constexpr std::size_t size() const { return 2; }

        value_type & operator[] (std::size_t i) { return value[i]; }
        value_type operator[] (std::size_t i) const { return value[i]; }

        template<typename R>
        t_vec2& operator = (const t_vec2<R>& rhs)
        {
            value[0] = rhs[0];
            value[1] = rhs[1];
            return *this;
        }

        T* data() { return value; }
        const T* data() const { return value; }
    };

    using vec2 = t_vec2<float>;
    using dvec2 = t_vec2<double>;

}

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
