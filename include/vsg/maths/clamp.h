#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/color.h>

#include <vulkan/vulkan.h>

namespace vsg
{

    /// clamp value between 0 and 1, implementing VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
    template<typename T>
    constexpr T clamp_to_edge(T value)
    {
        return value <= 0.0 ? 0.0 : value >= 1.0 ? 1.0 : value;
    }

    /// clamp value between 0 and 1, implementing VK_SAMPLER_ADDRESS_MODE_REPEAT
    template<typename T>
    constexpr T repeat(T value)
    {
        return value - std::floor(value);
    }

    /// clamp value between 0 and 1, implementing VK_SAMPLER_ADDRESS_MODE_MIRROR_REPEAT
    template<typename T>
    constexpr T mirror_repeat(T value)
    {
        T half_value = (std::abs(value) * 0.5);
        T v_fract = half_value - std::floor(half_value);
        return 1.0-std::abs(1.0-v_fract*2.0);
    }

    /// clamp value to range, return true if succeds.
    bool clamp(VkSamplerAddressMode mode, float& coord)
    {
        switch(mode)
        {
            case(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE):
                coord = clamp_to_edge(coord);
                return true;
            case(VK_SAMPLER_ADDRESS_MODE_REPEAT):
                coord = repeat(coord);
                return true;
            case(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT):
                coord = repeat(coord);
                return true;
            case(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER):
                if (coord < 0.0f) return false;
                if (coord > 1.0f) return false;
                return true;
            default:
                // TODO, not yet supported
                break;
        }
        return true;
    }

} // namespace vsg
