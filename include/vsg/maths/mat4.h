#pragma once

namespace vsg
{

template<typename T>
struct tmat4
{
    using value_type = T;

    value_type  data[4][4] = {
        { value_type(1), value_type(0), value_type(0), value_type(0) },
        { value_type(0), value_type(1), value_type(0), value_type(0) },
        { value_type(0), value_type(0), value_type(1), value_type(0) },
        { value_type(0), value_type(0), value_type(0), value_type(1) }
    };


    std::size_t size() const { return 16; }
    std::size_t columns() const { return 4; }
    std::size_t rows() const { return 4; }

    value_type & operator() (std::size_t i, std::size_t j) { return data[i][j]; }
    value_type operator() (std::size_t i, std::size_t j) const { return data[i][j]; }
};

using mat4 = tmat4<float>;
using dmat4 = tmat4<double>;

}