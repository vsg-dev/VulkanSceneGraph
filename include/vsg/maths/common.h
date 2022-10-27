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
    constexpr float PIf = 3.14159265358979323846f;
    constexpr double PI = 3.14159265358979323846;

    /// convert degrees to radians
    constexpr float radians(float degrees) noexcept { return degrees * (PIf / 180.0f); }
    constexpr double radians(double degrees) noexcept { return degrees * (PI / 180.0); }

    /// convert radians to degrees
    constexpr float degrees(float radians) noexcept { return radians * (180.0f / PIf); }
    constexpr double degrees(double radians) noexcept { return radians * (180.0 / PI); }

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
        double r = (x - edge0) / (edge1 - edge0);
        return edge0 + (r * r * (3.0 - 2.0 * r)) * (edge1 - edge0);
    }

    /// Hermite interpolation between 0.0 and 1.0
    template<typename T>
    T smoothstep(T r)
    {
        if (r <= 0.0)
            return 0.0;
        else if (r >= 1.0)
            return 1.0;
        return r * r * (3.0 - 2.0 * r);
    }

    /// interpolate between two values
    template<typename T>
    T mix(T start, T end, T r)
    {
        T one_minus_r = 1.0 - r;
        return start * one_minus_r + end * r;
    }

} // namespace vsg
