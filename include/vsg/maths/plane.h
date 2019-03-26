#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/sphere.h>

namespace vsg
{
    /** plane template class representing the plane in Hessian Normal Form : n.x = -p.*/
    template<typename T>
    struct t_plane
    {
        using value_type = T;
        using vec_type = t_vec3<T>;

        t_plane() :
            n{0.0, 0.0, 0.0},
            p(0.0) {}

        t_plane(value_type nx, value_type ny, value_type nz, value_type in_p) :
            n(nx, ny, nz),
            p(in_p) {}

        t_plane(const vec_type& normal, value_type in_p) :
            n(normal),
            p(in_p) {}

        t_plane(const vec_type& position, const vec_type& normal) :
            n(normal),
            p(position * normal) {}

        constexpr std::size_t size() const { return 4; }

        bool valid() const { return n.x!=0.0 && n.y!=0.0 && n.z!=0.0; }

        T* data() { return n.data(); }
        const T* data() const { return n.data(); }

        /// normal
        vec_type n;

        /// distance from origin
        value_type p;
    };

    using plane = t_plane<float>;
    using dplane = t_plane<double>;

    VSG_type_name(vsg::plane);
    VSG_type_name(vsg::dplane);

    template<typename T>
    constexpr T distance(t_plane<T> const& pl, t_vec3<T> const& v)
    {
        return dot(pl.n, v) - pl.p;
    }

    /** return true if bounding sphere is wholly or partially intersects with convex polytope defined by a list of planes with normals pointing inwards towards center of the polytope. */
    template<class PlaneItr, typename T>
    constexpr bool intersect(PlaneItr first, PlaneItr last, t_sphere<T> const& s)
    {
        auto negative_radius = -s.radius;
        for(auto itr = first; itr!=last; ++itr)
        {
            if (distance(*itr, s.center) < negative_radius) return false;
        }
        return true;
    }

    template<class Polytope, typename T>
    constexpr bool intersect(Polytope const& polytope, t_sphere<T> const& s)
    {
        return intersect(polytope.begin(), polytope.end(), s);
    }
} // namespace vsg
