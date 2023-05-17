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

#include <vsg/maths/sphere.h>

namespace vsg
{
    /** plane template class representing the plane in Hessian Normal Form : n.x = -p.*/
    template<typename T>
    struct t_plane
    {
        using value_type = T;
        using vec_type = t_vec4<T>;
        using normal_type = t_vec3<T>;

        union
        {
            value_type value[4];

            vec_type vec;

            // Hessian Normal Form
            struct
            {
                normal_type n;
                value_type p;
            };
        };

        constexpr t_plane() :
            value{0.0, 0.0, 0.0, 0.0} {}

        constexpr t_plane(const t_plane& pl) :
            value{pl[0], pl[1], pl[2], pl[3]} {}

        constexpr t_plane& operator=(const t_plane&) = default;

        constexpr explicit t_plane(const t_vec4<T>& v) :
            value{v[0], v[1], v[2], v[3]} {}

        constexpr t_plane(value_type nx, value_type ny, value_type nz, value_type in_p) :
            value{nx, ny, nz, in_p} {}

        constexpr t_plane(const normal_type& normal, value_type in_p) :
            value{normal.x, normal.y, normal.z, in_p} {}

        constexpr t_plane(const normal_type& position, const normal_type& normal) :
            value{normal.x, normal.y, normal.z, -(position * normal)} {}

        template<typename R>
        constexpr explicit t_plane(const t_plane<R>& v) :
            value{v[0], v[1], v[2], v[3]} {}

        template<typename R>
        constexpr explicit t_plane(const t_vec4<T>& v) :
            value{v[0], v[1], v[2], v[3]} {}

        constexpr std::size_t size() const { return 4; }

        value_type& operator[](std::size_t i) { return value[i]; }
        value_type operator[](std::size_t i) const { return value[i]; }

        template<typename R>
        t_plane& operator=(const t_plane<R>& rhs)
        {
            value[0] = static_cast<value_type>(rhs[0]);
            value[1] = static_cast<value_type>(rhs[1]);
            value[2] = static_cast<value_type>(rhs[2]);
            value[3] = static_cast<value_type>(rhs[3]);
            return *this;
        }

        void set(value_type in_x, value_type in_y, value_type in_z, value_type in_d)
        {
            value[0] = in_x;
            value[1] = in_y;
            value[2] = in_z;
            value[3] = in_d;
        }

        bool valid() const { return n.x != 0.0 && n.y != 0.0 && n.z != 0.0; }

        explicit operator bool() const noexcept { return valid(); }

        T* data() { return value; }
        const T* data() const { return value; }
    };

    using plane = t_plane<float>;
    using dplane = t_plane<double>;

    VSG_type_name(vsg::plane);
    VSG_type_name(vsg::dplane);

    template<typename T>
    constexpr T distance(const t_plane<T>& pl, const t_vec3<T>& v)
    {
        return dot(pl.n, v) + pl.p;
    }

    template<typename T, typename R>
    constexpr T distance(const t_plane<T>& pl, const t_vec3<R>& v)
    {
        using normal_type = typename t_plane<T>::normal_type;
        return dot(pl.n, normal_type(v)) + pl.p;
    }

    /** return true if bounding sphere is wholly or partially intersects with convex polytope defined by a list of planes with normals pointing inwards towards center of the polytope. */
    template<class PlaneItr, typename T>
    constexpr bool intersect(PlaneItr first, PlaneItr last, const t_sphere<T>& s)
    {
        auto negative_radius = -s.radius;
        for (auto itr = first; itr != last; ++itr)
        {
            if (distance(*itr, s.center) < negative_radius) return false;
        }
        return true;
    }

    template<class Polytope, typename T>
    constexpr bool intersect(const Polytope& polytope, const t_sphere<T>& s)
    {
        return intersect(polytope.begin(), polytope.end(), s);
    }
} // namespace vsg

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
