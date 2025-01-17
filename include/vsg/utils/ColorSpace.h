#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Chris Djali

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Array.h>
#include <vsg/core/Inherit.h>
#include <vsg/maths/color.h>

namespace vsg
{

    enum ColorSpace
    {
        sRGB,
        linearRGB
    };


    template<typename T>
    constexpr T linear_to_sRGB_component(T c)
    {
        constexpr T cutoff = static_cast<T>(0.04045 / 12.92);
        constexpr T linearFactor = static_cast<T>(12.92);
        constexpr T nonlinearFactor = static_cast<T>(1.055);
        constexpr T exponent = static_cast<T>(1.0 / 2.4);
        if (c <= cutoff)
            return c * linearFactor;
        else
            return std::pow(c, exponent) * nonlinearFactor - static_cast<T>(0.055);
    }

    template<typename T>
    constexpr T sRGB_to_linear_component(T c)
    {
        constexpr T cutoff = static_cast<T>(0.04045);
        constexpr T linearFactor = static_cast<T>(1.0 / 12.92);
        constexpr T nonlinearFactor = static_cast<T>(1.0 / 1.055);
        constexpr T exponent = static_cast<T>(2.4);
        if (c <= cutoff)
            return c * linearFactor;
        else
            return std::pow((c + static_cast<T>(0.055)) * nonlinearFactor, exponent);
    }

    template<typename T>
    constexpr t_vec4<T> linear_to_sRGB(const t_vec4<T>& src)
    {
        return t_vec4<T>(linear_to_sRGB_component(src.r), linear_to_sRGB_component(src.g), linear_to_sRGB_component(src.b), src.a);
    }

    template<typename T>
    constexpr t_vec4<T> linear_to_sRGB(T r, T g, T b, T a)
    {
        return t_vec4<T>(linear_to_sRGB_component(r), linear_to_sRGB_component(g), linear_to_sRGB_component(b), a);
    }

    template<typename T>
    constexpr t_vec4<T> sRGB_to_linear(const t_vec4<T>& src)
    {
        return t_vec4<T>(sRGB_to_linear_component(src.r), sRGB_to_linear_component(src.g), sRGB_to_linear_component(src.b), src.a);
    }

    template<typename T>
    constexpr t_vec4<T> sRGB_to_linear(T r, T g, T b, T a)
    {
        return t_vec4<T>(sRGB_to_linear_component(r), sRGB_to_linear_component(g), sRGB_to_linear_component(b), a);
    }

    template<typename T>
    constexpr t_vec4<T> linear_to_sRGB_approx(const t_vec4<T>& src)
    {
        const T exponent = static_cast<T>(1.0 / 2.2);
        return t_vec4<T>(std::pow(src.r, exponent), std::pow(src.g, exponent), std::pow(src.b, exponent), src.a);
    }

    template<typename T>
    constexpr t_vec4<T> linear_to_sRGB_approx(T r, T g, T b, T a)
    {
        const T exponent = static_cast<T>(1.0 / 2.2);
        return t_vec4<T>(std::pow(r, exponent), std::pow(g, exponent), std::pow(b, exponent), a);
    }

    template<typename T>
    constexpr t_vec4<T> sRGB_to_linear_approx(const t_vec4<T>& src)
    {
        const T exponent = static_cast<T>(2.2);
        return t_vec4<T>(std::pow(src.r, exponent), std::pow(src.g, exponent), std::pow(src.b, exponent), src.a);
    }

    template<typename T>
    constexpr t_vec4<T> sRGB_to_linear_approx(T r, T g, T b, T a)
    {
        const T exponent = static_cast<T>(2.2);
        return t_vec4<T>(std::pow(r, exponent), std::pow(g, exponent), std::pow(b, exponent), a);
    }

    template<typename T>
    void convert(size_t num, T* data, ColorSpace source, ColorSpace target)
    {
        if (source==sRGB && target==linearRGB) for(size_t i=0; i<num; ++i) data[i] = sRGB_to_linear(data[i]);
        else if (source==linearRGB && target==sRGB) for(size_t i=0; i<num; ++i) data[i] = linear_to_sRGB(data[i]);
    }

    extern VSG_DECLSPEC VkFormat uNorm_to_sRGB(VkFormat format);
    extern VSG_DECLSPEC VkFormat sRGB_to_uNorm(VkFormat format);

} // namespace vsg
