#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

// we can't implement the anonymous union/structs combination without causing warnings, so disable them for just this header
#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#endif
#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#    pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

#include <vsg/maths/vec3.h>

namespace vsg
{

    /// ucoord class for managing astronomical scale coordinates
    struct ucoord
    {
        using value_type = double;
        using vec_type = t_vec3<value_type>;

        vec_type origin;
        vec_type offset;

        constexpr ucoord() {}

        constexpr ucoord(const ucoord& uc) = default;
        constexpr ucoord(value_type in_x, value_type in_y, value_type in_z) :
            origin(0.0, 0.0, 0.0),
            offset(in_x, in_y, in_z) {}
        template<typename R>
        constexpr explicit ucoord(const t_vec3<R>& v) :
            origin(0.0, 0.0, 0.0),
            offset(static_cast<value_type>(v.x), static_cast<value_type>(v.y), static_cast<value_type>(v.z)) {}
        template<typename R>
        constexpr ucoord(const t_vec3<R>& in_origin, const t_vec3<R>& in_offset) :
            origin(in_origin),
            offset(in_offset){}

        ucoord& operator=(const ucoord& rhs)
        {
            origin = rhs.origin;
            offset = rhs.offset;
            return *this;
        }

        void set(value_type in_x, value_type in_y, value_type in_z)
        {
            origin.set(in_x, in_y, in_z);
            offset.set(in_x, in_y, in_z);
        }

        inline ucoord& operator+=(const ucoord& rhs)
        {
            origin += rhs.origin;
            offset += rhs.offset;
            return *this;
        }

        inline ucoord& operator-=(const ucoord& rhs)
        {
            origin -= rhs.origin;
            offset -= rhs.offset;
            return *this;
        }

        explicit operator bool() const noexcept { return origin || offset; }
    };

    VSG_type_name(vsg::ucoord);

    inline constexpr bool operator==(const ucoord& lhs, const ucoord& rhs)
    {
        return lhs.origin == rhs.origin && lhs.offset == rhs.offset;
    }

    inline constexpr bool operator!=(const ucoord& lhs, const ucoord& rhs)
    {
        return lhs.origin != rhs.origin || lhs.offset != rhs.offset;
    }

    inline bool operator<(const ucoord& lhs, const ucoord& rhs)
    {
        ucoord::vec_type delta = (rhs.origin-lhs.origin) + (rhs.offset - lhs.offset);
        if (delta[0] < 0.0) return true;
        if (delta[0] > 0.0) return false;
        if (delta[1] < 0.0) return true;
        if (delta[1] > 0.0) return false;
        return (delta[2] < 0.0);
    }

    inline constexpr ucoord operator-(const ucoord& lhs, const ucoord& rhs)
    {
        return ucoord(lhs.origin - rhs.origin, lhs.offset - rhs.offset);
    }

    inline constexpr ucoord operator+(const ucoord& lhs, const ucoord& rhs)
    {
        return ucoord(lhs.origin + rhs.origin, lhs.offset + rhs.offset);
    }

    inline constexpr ucoord::value_type length(const ucoord& uc)
    {
        return length(uc.origin + uc.offset);
    }

    inline constexpr ucoord::value_type length2(const ucoord& uc)
    {
        return length2(uc.origin + uc.offset);
    }

    constexpr ucoord mix(const ucoord& start, const ucoord& end, ucoord::value_type r)
    {
        ucoord::value_type one_minus_r = 1.0 - r;
        return ucoord(start.origin * one_minus_r + start.origin * r,
                      start.offset * one_minus_r + end.offset * r);
    }


} // namespace vsg

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
