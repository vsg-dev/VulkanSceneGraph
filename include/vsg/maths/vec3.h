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

    tvec3() : data{value_type(), value_type(), value_type()} {}
    tvec3(value_type x, value_type y, value_type z) : data{x, y, z} {}

    std::size_t size() const { return 3; }

    value_type & operator[] (std::size_t i) { return data[i]; }
    value_type operator[] (std::size_t i) const { return data[i]; }
};

using vec3f = tvec3<float>;
using vec3d = tvec3<double>;
using vec3 = vec3f;

}