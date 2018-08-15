#pragma once


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
    struct tvec2
    {
        using value_type = T;

        union
        {
            value_type  value[2];
            struct { value_type x, y; };
            struct { value_type r, g; };
            struct { value_type s, t; };
        };

        tvec2() : value{} {}
        tvec2(value_type in_x, value_type in_y) : value{in_x, in_y} {}

        std::size_t size() const { return 2; }

        value_type & operator[] (std::size_t i) { return value[i]; }
        value_type operator[] (std::size_t i) const { return value[i]; }

        template<typename R>
        tvec2& operator = (const tvec2<R>& rhs)
        {
            value[0] = rhs[0];
            value[1] = rhs[1];
            return *this;
        }

        T* data() { return value; }
        const T* data() const { return value; }
    };

    using vec2 = tvec2<float>;
    using dvec2 = tvec2<double>;

}

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
