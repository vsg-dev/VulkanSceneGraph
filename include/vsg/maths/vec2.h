#pragma once

namespace vsg
{

template<typename T>
struct tvec2
{
    using value_type = T;

    union
    {
        value_type  data[2];
        struct { value_type x, y; };
        struct { value_type r, g; };
        struct { value_type s, t; };
    };

    tvec2() : data{} {}
    tvec2(value_type x, value_type y) : data{x, y} {}

    std::size_t size() const { return 2; }

    value_type & operator[] (std::size_t i) { return data[i]; }
    value_type operator[] (std::size_t i) const { return data[i]; }
};

using vec2 = tvec2<float>;
using dvec2 = tvec2<double>;

}