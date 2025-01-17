#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/vec4.h>

namespace vsg
{
    template<typename T>
    constexpr T color_cast(float r, float g, float b, float a)
    {
        return {r, g, b, a};
    }

    template<>
    constexpr float color_cast<float>(float r, float, float, float)
    {
        return r;
    }

    template<>
    constexpr vec2 color_cast<vec2>(float r, float g, float, float)
    {
        return {r, g};
    }

    template<>
    constexpr vec3 color_cast<vec3>(float r, float g, float b, float)
    {
        return {r, g, b};
    }

    template<>
    constexpr vec4 color_cast<vec4>(float r, float g, float b, float a)
    {
        return {r, g, b, a};
    }

    template<>
    constexpr ubvec4 color_cast<ubvec4>(float r, float g, float b, float a)
    {
        return {static_cast<uint8_t>(r * 255.0f), static_cast<uint8_t>(g * 255.0f), static_cast<uint8_t>(b * 255.0f), static_cast<uint8_t>(a * 255.0f)};
    }

    template<typename T>
    constexpr T transparent_black()
    {
        return color_cast<T>(0.0f, 0.0f, 0.0f, 0.0f);
    }

    template<typename T>
    constexpr T opaque_black()
    {
        return color_cast<T>(0.0f, 0.0f, 0.0f, 1.0f);
    }

    template<typename T>
    constexpr T transparent_white()
    {
        return color_cast<T>(1.0f, 1.0f, 1.0f, 0.0f);
    }

    template<typename T>
    constexpr T opaque_white()
    {
        return color_cast<T>(0.0f, 0.0f, 1.0f, 1.0f);
    }

    template<typename T>
    constexpr T color_cast(VkBorderColor borderColor)
    {
        switch (borderColor)
        {
        case (VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK):
        case (VK_BORDER_COLOR_INT_TRANSPARENT_BLACK):
            return transparent_black<T>();
        case (VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK):
        case (VK_BORDER_COLOR_INT_OPAQUE_BLACK):
            return opaque_black<T>();
        case (VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE):
        case (VK_BORDER_COLOR_INT_OPAQUE_WHITE):
            return opaque_white<T>();
        default:
            // not supported, fallback to VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK
            return transparent_black<T>();
        }
    }

} // namespace vsg
