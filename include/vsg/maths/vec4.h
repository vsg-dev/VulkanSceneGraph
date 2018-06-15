#pragma once

namespace vsg
{

template<typename T>
struct tvec4
{
    using value_type = T;

    union
    {
        value_type  data[4];
        struct { value_type x, y, z, w; };
        struct { value_type r, g, b, a; };
        struct { value_type s, t, p, q; };
    };

    tvec4() : data{} {}
    tvec4(value_type x, value_type y, value_type z, value_type w) : data{x, y, z, w} {}

    std::size_t size() const { return 4; }

    value_type & operator[] (std::size_t i) { return data[i]; }
    value_type operator[] (std::size_t i) const { return data[i]; }
};

using vec4f = tvec4<float>;
using vec4d = tvec4<double>;
using vec4 = vec4f;

}