#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/vec3.h>

#include <limits>

namespace vsg
{
    /// t_box template class that represent a axis aligned bounding box
    template<typename T>
    struct t_box
    {
        using value_type = T;
        using vec_type = t_vec3<T>;

        vec_type min = vec_type(std::numeric_limits<value_type>::max(), std::numeric_limits<value_type>::max(), std::numeric_limits<value_type>::max());
        vec_type max = vec_type(std::numeric_limits<value_type>::lowest(), std::numeric_limits<value_type>::lowest(), std::numeric_limits<value_type>::lowest());

        constexpr t_box() = default;
        constexpr t_box(const t_box& s) = default;

        template<typename R>
        constexpr explicit t_box(const t_box<R>& s) :
            min(s.min),
            max(s.max) {}

        constexpr t_box(const vec_type& in_min, const vec_type& in_max) :
            min(in_min),
            max(in_max) {}

        constexpr t_box& operator=(const t_box&) = default;

        constexpr std::size_t size() const { return 6; }

        value_type& operator[](std::size_t i) { return data()[i]; }
        value_type operator[](std::size_t i) const { return data()[i]; }

        bool valid() const { return min.x <= max.x; }

        T* data() { return min.data(); }
        const T* data() const { return min.data(); }

        template<typename R>
        void add(const t_vec3<R>& v)
        {
            add(v.x, v.y, v.z);
        }

        void add(value_type x, value_type y, value_type z)
        {
            if (x < min.x) min.x = x;
            if (y < min.y) min.y = y;
            if (z < min.z) min.z = z;
            if (x > max.x) max.x = x;
            if (y > max.y) max.y = y;
            if (z > max.z) max.z = z;
        }

        template<typename R>
        void add(const t_box<R>& bb)
        {
            if (bb.min.x < min.x) min.x = bb.min.x;
            if (bb.min.y < min.y) min.y = bb.min.y;
            if (bb.min.z < min.z) min.z = bb.min.z;
            if (bb.max.x > max.x) max.x = bb.max.x;
            if (bb.max.y > max.y) max.y = bb.max.y;
            if (bb.max.z > max.z) max.z = bb.max.z;
        }
    };

    using box = t_box<float>;   /// float box class
    using dbox = t_box<double>; /// double box class

    VSG_type_name(vsg::box);
    VSG_type_name(vsg::dbox);
} // namespace vsg
