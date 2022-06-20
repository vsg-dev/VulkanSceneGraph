#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/color.h>

namespace vsg
{

    /// clamp value between 0 and 1, implementing VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
    template<typename T>
    constexpr T clamp_to_edge(T value)
    {
        return value <= T(0.0) ? T(0.0) : value >= T(1.0) ? T(1.0)
                                                          : value;
    }

    /// clamp value between 0 and 1, implementing VK_SAMPLER_ADDRESS_MODE_REPEAT
    template<typename T>
    constexpr T repeat(T value)
    {
        T result = value - std::floor(value);
        if (result != T(0.0)) return result;
        return (value > T(0.0)) ? T(1.0) : T(0.0);
    }

    /// clamp value between 0 and 1, implementing VK_SAMPLER_ADDRESS_MODE_MIRROR_REPEAT
    template<typename T>
    constexpr T mirror_repeat(T value)
    {
        T half_value = (std::abs(value) * T(0.5));
        T v_fract = half_value - std::floor(half_value);
        return T(1.0) - std::abs(T(1.0) - v_fract * T(2.0));
    }

    /// clamp value to range, return true if succeeds.
    inline bool clamp(VkSamplerAddressMode mode, float& coord)
    {
        switch (mode)
        {
        case (VK_SAMPLER_ADDRESS_MODE_REPEAT):
            coord = repeat(coord);
            return true;
        case (VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT):
            coord = mirror_repeat(coord);
            return true;
        case (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE):
            coord = clamp_to_edge(coord);
            return true;
        case (VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER):
            if (coord < 0.0f) return false;
            if (coord > 1.0f) return false;
            return true;
        default:
            // not supported, fallback to clamp_to_edge
            coord = clamp_to_edge(coord);
            break;
        }
        return true;
    }

} // namespace vsg
