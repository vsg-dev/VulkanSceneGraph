#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <string>
#include <typeinfo>

namespace vsg
{
    // clang-format off

    template<typename T>
    constexpr const char* type_name() noexcept { return typeid(T).name(); }

    template<typename T>
    constexpr const char* type_name(const T&) noexcept { return type_name<T>(); }

    template<> constexpr const char* type_name<std::string>() noexcept { return "string"; }
    template<> constexpr const char* type_name<bool>() noexcept { return "bool"; }
    template<> constexpr const char* type_name<char>() noexcept { return "char"; }
    template<> constexpr const char* type_name<int8_t>() noexcept { return "int8_t"; }
    template<> constexpr const char* type_name<unsigned char>() noexcept { return "uchar"; }
    template<> constexpr const char* type_name<short>() noexcept { return "short"; }
    template<> constexpr const char* type_name<unsigned short>() noexcept { return "ushort"; }
    template<> constexpr const char* type_name<int>() noexcept { return "int"; }
    template<> constexpr const char* type_name<unsigned int>() noexcept { return "uint"; }
    template<> constexpr const char* type_name<float>() noexcept { return "float"; }
    template<> constexpr const char* type_name<double>() noexcept { return "double"; }

    // helper define for defining the type_name() for a type within the vsg namespace.
    #define VSG_type_name(T) \
        template<> constexpr const char* type_name<T>() noexcept { return #T; } \
        template<> constexpr const char* type_name<const T>() noexcept { return "const "#T; }


    // helper define for defining the type_name() for a type in a namespace other than vsg, note must be placed in global namespace.
    #define EVSG_type_name(T) \
        template<> constexpr const char* vsg::type_name<T>() noexcept { return #T; } \
        template<> constexpr const char* vsg::type_name<const T>() noexcept { return "const "#T; }

    // clang-format on

} // namespace vsg
