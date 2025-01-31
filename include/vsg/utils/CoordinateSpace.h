#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Chris Djali

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/color.h>

namespace vsg
{

    enum class CoordinateSpace
    {
        NO_PREFERENCE = 0,
        LINEAR = (1 << 0),
        sRGB = (1 << 1)
    };
    VSG_type_name(vsg::CoordinateSpace);

    template<typename T>
    constexpr T linear_to_sRGB(T c)
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
    constexpr T sRGB_to_linear(T c)
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
    constexpr t_vec3<T> linear_to_sRGB(const t_vec3<T>& src)
    {
        return t_vec3<T>(linear_to_sRGB(src.r), linear_to_sRGB(src.g), linear_to_sRGB(src.b));
    }

    template<typename T>
    constexpr t_vec4<T> linear_to_sRGB(const t_vec4<T>& src)
    {
        return t_vec4<T>(linear_to_sRGB(src.r), linear_to_sRGB(src.g), linear_to_sRGB(src.b), src.a);
    }

    template<typename T>
    constexpr t_vec3<T> linear_to_sRGB(T r, T g, T b)
    {
        return t_vec3<T>(linear_to_sRGB(r), linear_to_sRGB(g), linear_to_sRGB(b));
    }

    template<typename T>
    constexpr t_vec3<T> linear_to_sRGB(T r, T g, T b, T a)
    {
        return t_vec4<T>(linear_to_sRGB(r), linear_to_sRGB(g), linear_to_sRGB(b), a);
    }

    template<typename T>
    constexpr t_vec3<T> sRGB_to_linear(const t_vec3<T>& src)
    {
        return t_vec3<T>(sRGB_to_linear(src.r), sRGB_to_linear(src.g), sRGB_to_linear(src.b));
    }

    template<typename T>
    constexpr t_vec4<T> sRGB_to_linear(const t_vec4<T>& src)
    {
        return t_vec4<T>(sRGB_to_linear(src.r), sRGB_to_linear(src.g), sRGB_to_linear(src.b), src.a);
    }

    template<typename T>
    constexpr t_vec3<T> sRGB_to_linear(T r, T g, T b)
    {
        return t_vec3<T>(sRGB_to_linear(r), sRGB_to_linear(g), sRGB_to_linear(b));
    }

    template<typename T>
    constexpr t_vec4<T> sRGB_to_linear(T r, T g, T b, T a)
    {
        return t_vec4<T>(sRGB_to_linear(r), sRGB_to_linear(g), sRGB_to_linear(b), a);
    }

    template<typename T>
    void convert(T& data, CoordinateSpace source, CoordinateSpace target)
    {
        if (source == CoordinateSpace::sRGB && target == CoordinateSpace::LINEAR)
            data = sRGB_to_linear(data);
        else if (source == CoordinateSpace::LINEAR && target == CoordinateSpace::sRGB)
            data = linear_to_sRGB(data);
    }

    template<typename T>
    void convert(size_t num, T* data, CoordinateSpace source, CoordinateSpace target)
    {
        if (source == CoordinateSpace::sRGB && target == CoordinateSpace::LINEAR)
            for (size_t i = 0; i < num; ++i) data[i] = sRGB_to_linear(data[i]);
        else if (source == CoordinateSpace::LINEAR && target == CoordinateSpace::sRGB)
            for (size_t i = 0; i < num; ++i) data[i] = linear_to_sRGB(data[i]);
    }

    extern VSG_DECLSPEC VkFormat uNorm_to_sRGB(VkFormat format);
    extern VSG_DECLSPEC VkFormat sRGB_to_uNorm(VkFormat format);

} // namespace vsg
