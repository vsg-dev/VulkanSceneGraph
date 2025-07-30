#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <limits>

namespace vsg
{
    template<typename T>
    struct numbers
    {
        static constexpr T zero() { return static_cast<T>(0.0); }
        static constexpr T half() { return static_cast<T>(0.5); }
        static constexpr T one() { return static_cast<T>(1.0); }
        static constexpr T two() { return static_cast<T>(2.0); }
        static constexpr T three() { return static_cast<T>(3.0); }

        static constexpr T minus_one() { return static_cast<T>(-1.0); }

        static constexpr T epsilon() { return std::numeric_limits<T>::epsilon(); }

        static constexpr T PI() { return static_cast<T>(3.14159265358979323846); }
        static constexpr T degrees_to_radians() { return PI() / static_cast<T>(180.0); }
        static constexpr T radians_to_degrees() { return static_cast<T>(180.0) / PI(); }
    };

} // namespace vsg
