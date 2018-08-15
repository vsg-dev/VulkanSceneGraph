#pragma once

#include <cmath>

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
        tvec3(value_type x, value_type y, value_type z) : value{x, y, z} {}

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
