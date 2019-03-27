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

namespace vsg
{
    template<typename T>
    struct t_sphere
    {
        using value_type = T;
        using vec_type = t_vec3<T>;

        union
        {
            value_type value[4];

            struct
            {
                value_type x, y, z, r;
            };

            struct
            {
                vec_type center;
                value_type radius;
            };
        };

        constexpr t_sphere() :
            value{0.0, 0.0, 0.0, -1.0} {}

        constexpr t_sphere(const t_sphere& s) :
            value{s[0], s[1], s[2], s[3]} {}

        constexpr t_sphere(const vec_type& c, value_type r) :
            value{c.x, c.y, c.z, r} {}

        constexpr std::size_t size() const { return 4; }

        value_type& operator[](std::size_t i) { return value[i]; }
        value_type operator[](std::size_t i) const { return value[i]; }

        template<typename R>
        t_sphere& operator=(const t_sphere<R>& rhs)
        {
            value[0] = static_cast<value_type>(rhs[0]);
            value[1] = static_cast<value_type>(rhs[1]);
            value[2] = static_cast<value_type>(rhs[2]);
            value[3] = static_cast<value_type>(rhs[3]);
            return *this;
        }

        bool valid() const { return radius >= 0.0; }

        T* data() { return value; }
        const T* data() const { return value; }
    };

    using sphere = t_sphere<float>;
    using dsphere = t_sphere<double>;

    VSG_type_name(vsg::sphere);
    VSG_type_name(vsg::dsphere);
} // namespace vsg

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
