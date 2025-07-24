#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ConstVisitor.h>
#include <vsg/core/visit.h>
#include <vsg/maths/mat3.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/quat.h>
#include <vsg/maths/vec3.h>

#include <cmath>

namespace vsg
{

    constexpr float PIf = numbers<float>::PI();
    constexpr double PI = numbers<double>::PI();

    /// convert degrees to radians
    constexpr float radians(float degrees) noexcept { return degrees * numbers<float>::degrees_to_radians(); }
    constexpr double radians(double degrees) noexcept { return degrees * numbers<double>::degrees_to_radians(); }

    /// convert radians to degrees
    constexpr float degrees(float radians) noexcept { return radians * numbers<float>::radians_to_degrees(); }
    constexpr double degrees(double radians) noexcept { return radians * numbers<double>::radians_to_degrees(); }

    /// compute value^2
    constexpr float square(float v) noexcept { return v * v; };
    constexpr double square(double v) noexcept { return v * v; };

    /// Hermite interpolation between edge0 and edge1
    template<typename T>
    T smoothstep(T edge0, T edge1, T x)
    {
        if (x <= edge0)
            return edge0;
        else if (x >= edge1)
            return edge1;
        T r = (x - edge0) / (edge1 - edge0);
        return edge0 + (r * r * (numbers<T>::three() - numbers<T>::two() * r)) * (edge1 - edge0);
    }

    /// Hermite interpolation between 0.0 and 1.0
    template<typename T>
    T smoothstep(T r)
    {
        if (r <= numbers<T>::zero())
            return numbers<T>::zero();
        else if (r >= numbers<T>::one())
            return numbers<T>::one();
        return r * r * (numbers<T>::three() - numbers<T>::two() * r);
    }

    /// interpolate between two values
    template<typename T>
    T mix(T start, T end, T r)
    {
        T one_minus_r = numbers<T>::one() - r;
        return start * one_minus_r + end * r;
    }

    /// return the number of bits supported by the long double implementation - VisualStudio by default 64 bits, stored as 8 bytes, & Linux x86_64 defaults to 80 bits stored as 16bytes, some CPU architectures support full 128 bits/16 bytes.
    extern VSG_DECLSPEC uint32_t native_long_double_bits();

} // namespace vsg
