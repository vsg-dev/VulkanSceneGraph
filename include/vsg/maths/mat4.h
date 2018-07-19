#pragma once

#include <vsg/maths/vec4.h>

namespace vsg
{

    template<typename T>
    class tmat4
    {
    public:
        using value_type = T;
        using column_type = tvec4<T>;

        column_type value[4];

        tmat4() : value{{1, 0, 0, 0},
                        {0, 1, 0, 0},
                        {0, 0, 1, 0},
                        {0, 0, 0, 1}} {}

        explicit tmat4(value_type v) : value{{v, 0, 0, 0},
                                             {0, v, 0, 0},
                                             {0, 0, v, 0},
                                             {0, 0, 0, v}} {}

        tmat4(value_type v0, value_type v1, value_type v2, value_type v3,
            value_type v4, value_type v5, value_type v6, value_type v7,
            value_type v8, value_type v9, value_type v10, value_type v11,
            value_type v12, value_type v13, value_type v14, value_type v15) :
            value{{v0, v4, v8,  v12},
                  {v1, v5, v9,  v13},
                  {v2, v6, v10, v14},
                  {v3, v7, v11, v15}} {}

        tmat4(value_type v[16]) :
            value{{v[0], v[4], v[8],  v[12]},
                  {v[1], v[5], v[9],  v[13]},
                  {v[2], v[6], v[10], v[14]},
                  {v[3], v[7], v[11], v[15]}} {}

        template<typename R>
        tmat4(const tmat4<R>& rhs)
        {
            value[0] = rhs[0];
            value[1] = rhs[1];
            value[2] = rhs[2];
            value[3] = rhs[3];
        }

        std::size_t size() const { return 16; }
        std::size_t columns() const { return 4; }
        std::size_t rows() const { return 4; }

        column_type& operator[] (std::size_t c) { return value[c]; }
        column_type const& operator[] (std::size_t c) const { return value[c]; }

        value_type & operator() (std::size_t c, std::size_t r) { return value[c][r]; }
        value_type operator() (std::size_t c, std::size_t r) const { return value[c][r]; }

        template<typename R>
        tmat4& operator = (const tmat4<R>& rhs)
        {
            value[0] = rhs[0];
            value[1] = rhs[1];
            value[2] = rhs[2];
            value[3] = rhs[3];
            return *this;
        }

        T* data() { return value[0].data(); }
        const T* data() const { return value[0].data(); }

    };

    using mat4 = tmat4<float>;
    using dmat4 = tmat4<double>;

}