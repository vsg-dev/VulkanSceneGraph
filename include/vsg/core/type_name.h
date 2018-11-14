#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <typeinfo>

namespace vsg
{
#if 0
    const char* type_name(const Object& object) noexcept { return object.className(); }
    constexpr const char* type_name(const std::string&) noexcept { return "string"; }
    constexpr const char* type_name(bool) noexcept { return "char"; }
    constexpr const char* type_name(char) noexcept { return "char"; }
    constexpr const char* type_name(unsigned char) noexcept { return "uchar"; }
    constexpr const char* type_name(short) noexcept { return "short"; }
    constexpr const char* type_name(unsigned short) noexcept { return "ushort"; }
    constexpr const char* type_name(int) noexcept { return "int"; }
    constexpr const char* type_name(unsigned int) noexcept { return "uint"; }
    constexpr const char* type_name(float) noexcept { return "float"; }
    constexpr const char* type_name(double) noexcept { return "double"; }

    constexpr const char* type_name(const vec2&) noexcept { return "vsg::vec2"; }
    constexpr const char* type_name(const vec3&) noexcept { return "vsg::vec3"; }
    constexpr const char* type_name(const vec4&) noexcept { return "vsg::vec4"; }
    constexpr const char* type_name(const mat4&) noexcept { return "vsg::mat4"; }
    constexpr const char* type_name(const dvec2&) noexcept { return "vsg::dvec2"; }
    constexpr const char* type_name(const dvec3&) noexcept { return "vsg::dvec3"; }
    constexpr const char* type_name(const dvec4&) noexcept { return "vsg::dvec4"; }
    constexpr const char* type_name(const dmat4&) noexcept { return "vsg::dmat4"; }

    template<typename T>
    constexpr const char* type_name(const T&) noexcept { return typeid(T).name(); }

#else

    template<typename T>
    constexpr const char* type_name() noexcept { return typeid(T).name(); }

    template<typename T>
    constexpr const char* type_name(const T&) noexcept { return type_name<T>(); }

    template<> constexpr const char* type_name<std::string>() noexcept { return "string"; }
    template<> constexpr const char* type_name<bool>()noexcept { return "char"; }
    template<> constexpr const char* type_name<char>()noexcept { return "char"; }
    template<> constexpr const char* type_name<unsigned char>()noexcept { return "uchar"; }
    template<> constexpr const char* type_name<short>()noexcept { return "short"; }
    template<> constexpr const char* type_name<unsigned short>()noexcept { return "ushort"; }
    template<> constexpr const char* type_name<int>()noexcept { return "int"; }
    template<> constexpr const char* type_name<unsigned int>()noexcept { return "uint"; }
    template<> constexpr const char* type_name<float>()noexcept { return "float"; }
    template<> constexpr const char* type_name<double>()noexcept { return "double"; }

    #define VSG_TYPE_NAME(T) template<> constexpr const char* type_name<T>() noexcept { return #T; }

    template<> constexpr const char* type_name<vec2>()noexcept { return "vsg::vec2"; }
    template<> constexpr const char* type_name<vec3>()noexcept { return "vsg::vec3"; }
    template<> constexpr const char* type_name<vec4>()noexcept { return "vsg::vec4"; }
    template<> constexpr const char* type_name<mat4>()noexcept { return "vsg::mat4"; }
    template<> constexpr const char* type_name<dvec2>()noexcept { return "vsg::dvec2"; }
    template<> constexpr const char* type_name<dvec3>()noexcept { return "vsg::dvec3"; }
    template<> constexpr const char* type_name<dvec4>()noexcept { return "vsg::dvec4"; }
    template<> constexpr const char* type_name<dmat4>()noexcept { return "vsg::dmat4"; }
#endif


} // namespace vsg
