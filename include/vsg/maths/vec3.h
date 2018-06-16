#pragma once

namespace vsg
{

template<typename T>
struct tvec3
{
    using value_type = T;

    union
    {
        value_type  data[3];
        struct { value_type x, y, z; };
        struct { value_type r, g, b; };
        struct { value_type s, t, p; };
    };

    tvec3() : data{} {}
    tvec3(value_type x, value_type y, value_type z) : data{x, y, z} {}

    std::size_t size() const { return 3; }

    value_type & operator[] (std::size_t i) { return data[i]; }
    value_type operator[] (std::size_t i) const { return data[i]; }
};

using vec3 = tvec3<float>;
using dvec3 = tvec3<double>;

}