#pragma once

namespace vsg
{

template<typename T>
struct tmat4
{
    using value_type = T;

    value_type  data[16] = {
        value_type(1), value_type(0), value_type(0), value_type(0),
        value_type(0), value_type(1), value_type(0), value_type(0),
        value_type(0), value_type(0), value_type(1), value_type(0),
        value_type(0), value_type(0), value_type(0), value_type(1)
    };


    std::size_t size() const { return 16; }
    std::size_t columns() const { return 4; }
    std::size_t rows() const { return 4; }

    value_type & operator() (std::size_t c, std::size_t r) { return data[c+r*4]; }
    value_type operator() (std::size_t c, std::size_t r) const { return data[c+r*4]; }
};

using mat4 = tmat4<float>;
using dmat4 = tmat4<double>;

}