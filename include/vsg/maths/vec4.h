#pragma once

namespace vsg
{

    template<typename T>
    struct tvec4
    {
        using value_type = T;

        union
        {
            value_type  value[4];
            struct { value_type x, y, z, w; };
            struct { value_type r, g, b, a; };
            struct { value_type s, t, p, q; };
        };

        tvec4() : value{} {}
        tvec4(value_type x, value_type y, value_type z, value_type w) : value{x, y, z, w} {}

        std::size_t size() const { return 4; }

        value_type & operator[] (std::size_t i) { return value[i]; }
        value_type operator[] (std::size_t i) const { return value[i]; }

        template<typename R>
        tvec4& operator = (const tvec4<R>& rhs)
        {
            value[0] = rhs[0];
            value[1] = rhs[1];
            value[2] = rhs[2];
            value[3] = rhs[3];
            return *this;
        }

        T* data() { return value; }
        const T* data() const { return value; }
    };

    using vec4 = tvec4<float>;
    using dvec4 = tvec4<double>;

}
