#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/core/type_name.h>
#include <vsg/io/Path.h>
#include <vsg/maths/box.h>
#include <vsg/maths/mat3.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/plane.h>
#include <vsg/maths/quat.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>
#include <vsg/utils/CoordinateSpace.h>

#include <istream>
#include <ostream>
#include <sstream>

namespace vsg
{

    /// helper class for inserting indentation into streams useful for formatting output.
    struct indentation
    {
        int indent = 0;

        indentation& operator+=(int delta)
        {
            indent += delta;
            return *this;
        }
        indentation& operator-=(int delta)
        {
            indent -= delta;
            return *this;
        }
    };

    inline indentation operator+(const indentation& lhs, const int rhs) { return indentation{lhs.indent + rhs}; }
    inline indentation operator-(const indentation& lhs, const int rhs) { return indentation{lhs.indent - rhs}; }

    inline std::ostream& operator<<(std::ostream& output, const indentation& in)
    {
        for (int i = 0; i < in.indent; ++i) output.put(' ');
        return output;
    }

    /// convenience function for writing/streaming values to a std::string
    template<typename... Args>
    std::string make_string(const Args&... args)
    {
        std::ostringstream stream;
        (stream << ... << args);
        return stream.str();
    }

    /// output stream support for vsg::t_vec2
    template<typename T>
    std::ostream& operator<<(std::ostream& output, const vsg::t_vec2<T>& vec)
    {
        output << vec.x << " " << vec.y;
        return output;
    }

    /// input stream support for vsg::t_vec2
    template<typename T>
    std::istream& operator>>(std::istream& input, vsg::t_vec2<T>& vec)
    {
        input >> vec.x >> vec.y;
        return input;
    }

    /// output stream support for vsg::t_vec3
    template<typename T>
    std::ostream& operator<<(std::ostream& output, const vsg::t_vec3<T>& vec)
    {
        output << vec.x << " " << vec.y << " " << vec.z;
        return output;
    }

    /// input stream support for vsg::t_vec3
    template<typename T>
    std::istream& operator>>(std::istream& input, vsg::t_vec3<T>& vec)
    {
        input >> vec.x >> vec.y >> vec.z;
        return input;
    }

    /// output stream support for vsg::t_vec4
    template<typename T>
    std::ostream& operator<<(std::ostream& output, const vsg::t_vec4<T>& vec)
    {
        output << vec.x << " " << vec.y << " " << vec.z << " " << vec.w;
        return output;
    }

    /// input stream support for vsg::t_vec4
    template<typename T>
    std::istream& operator>>(std::istream& input, vsg::t_vec4<T>& vec)
    {
        input >> vec.x >> vec.y >> vec.z >> vec.w;
        return input;
    }

    /// output stream support for vsg:t_quat
    template<typename T>
    std::ostream& operator<<(std::ostream& output, const vsg::t_quat<T>& q)
    {
        output << q.x << " " << q.y << " " << q.z << " " << q.w;
        return output;
    }

    /// input stream support for vsg:t_quat
    template<typename T>
    std::istream& operator>>(std::istream& input, vsg::t_quat<T>& q)
    {
        input >> q.x >> q.y >> q.z >> q.w;
        return input;
    }

    /// output stream support for vsg::t_plane
    template<typename T>
    std::ostream& operator<<(std::ostream& output, const vsg::t_plane<T>& vec)
    {
        output << vec.value[0] << " " << vec.value[1] << " " << vec.value[2] << " " << vec.value[3];
        return output;
    }

    /// input stream support for vsg::t_plane
    template<typename T>
    std::istream& operator>>(std::istream& input, vsg::t_plane<T>& vec)
    {
        input >> vec.value[0] >> vec.value[1] >> vec.value[2] >> vec.value[3];
        return input;
    }

    /// output stream support for vsg::t_mat3
    template<typename T>
    std::ostream& operator<<(std::ostream& output, const vsg::t_mat3<T>& mat)
    {
        output << std::endl;
        output << "    " << mat(0, 0) << " " << mat(1, 0) << " " << mat(2, 0) << std::endl;
        output << "    " << mat(0, 1) << " " << mat(1, 1) << " " << mat(2, 1) << std::endl;
        output << "    " << mat(0, 2) << " " << mat(1, 2) << " " << mat(2, 2) << std::endl;
        return output;
    }

    /// input stream support for vsg::t_mat3
    template<typename T>
    std::istream& operator>>(std::istream& input, vsg::t_mat3<T>& mat)
    {
        input >> mat(0, 0) >> mat(1, 0) >> mat(2, 0);
        input >> mat(0, 1) >> mat(1, 1) >> mat(2, 1);
        input >> mat(0, 2) >> mat(1, 2) >> mat(2, 2);
        return input;
    }

    /// output stream support for vsg::t_mat4
    template<typename T>
    std::ostream& operator<<(std::ostream& output, const vsg::t_mat4<T>& mat)
    {
        output << std::endl;
        output << "    " << mat(0, 0) << " " << mat(1, 0) << " " << mat(2, 0) << " " << mat(3, 0) << std::endl;
        output << "    " << mat(0, 1) << " " << mat(1, 1) << " " << mat(2, 1) << " " << mat(3, 1) << std::endl;
        output << "    " << mat(0, 2) << " " << mat(1, 2) << " " << mat(2, 2) << " " << mat(3, 2) << std::endl;
        output << "    " << mat(0, 3) << " " << mat(1, 3) << " " << mat(2, 3) << " " << mat(3, 3) << std::endl;
        return output;
    }

    /// input stream support for vsg::t_mat4
    template<typename T>
    std::istream& operator>>(std::istream& input, vsg::t_mat4<T>& mat)
    {
        input >> mat(0, 0) >> mat(1, 0) >> mat(2, 0) >> mat(3, 0);
        input >> mat(0, 1) >> mat(1, 1) >> mat(2, 1) >> mat(3, 1);
        input >> mat(0, 2) >> mat(1, 2) >> mat(2, 2) >> mat(3, 2);
        input >> mat(0, 3) >> mat(1, 3) >> mat(2, 3) >> mat(3, 3);
        return input;
    }

    /// output stream support for vsg::t_sphere
    template<typename T>
    std::ostream& operator<<(std::ostream& output, const vsg::t_sphere<T>& sp)
    {
        output << sp.vec;
        return output;
    }

    /// input stream support for vsg::t_sphere
    template<typename T>
    std::istream& operator>>(std::istream& input, vsg::t_sphere<T>& sp)
    {
        input >> sp.vec;
        return input;
    }

    /// output stream support for vsg::t_box
    template<typename T>
    std::ostream& operator<<(std::ostream& output, const vsg::t_box<T>& bx)
    {
        output << std::endl;
        output << "    " << bx.min << std::endl;
        output << "    " << bx.max << std::endl;
        return output;
    }

    /// input stream support for vsg::t_box
    template<typename T>
    std::istream& operator>>(std::istream& input, vsg::t_box<T>& bx)
    {
        input >> bx.min;
        input >> bx.max;
        return input;
    }

    /// output stream support for vsg::ref_ptr
    template<typename T>
    std::ostream& operator<<(std::ostream& output, const vsg::ref_ptr<T>& ptr)
    {
        if (ptr)
            output << "ref_ptr<" << vsg::type_name<T>() << ">(" << ptr->className() << " " << ptr.get() << ")";
        else
            output << "ref_ptr<" << vsg::type_name<T>() << ">(nullptr)";
        return output;
    }

    /// output stream support for std::pair
    template<typename T, typename R>
    std::ostream& operator<<(std::ostream& output, const std::pair<T, R>& wd)
    {
        output << wd.first << " " << wd.second;
        return output;
    }

    /// input stream support for std::pair
    template<typename T, typename R>
    std::istream& operator>>(std::istream& input, std::pair<T, R>& wd)
    {
        input >> wd.first >> wd.second;
        return input;
    }

    /// output stream support for enums
    template<typename T>
    std::ostream& operator<<(typename std::enable_if<std::is_enum<T>::value, std::ostream>::type& stream, const T& e)
    {
        return stream << static_cast<typename std::underlying_type<T>::type>(e);
    }

    /// output stream support for vsg::Path
    inline std::ostream& operator<<(std::ostream& output, const vsg::Path& path)
    {
        output << path.string();
        return output;
    }

    /// input stream support for vsg::Path
    inline std::istream& operator>>(std::istream& input, vsg::Path& path)
    {
        std::string str;
        input >> str;
        path = str;
        return input;
    }

    /// output stream support for vsg::Exception
    inline std::ostream& operator<<(std::ostream& output, const vsg::Exception& e)
    {
        output << "Error code: " << e.result << " | " << e.message;
        return output;
    }

    inline std::istream& operator>>(std::istream& input, CoordinateSpace& coordinateSpace)
    {
        std::string str;
        input >> str;

        if (str == "LINEAR")
            coordinateSpace = CoordinateSpace::LINEAR;
        else if (str == "sRGB")
            coordinateSpace = CoordinateSpace::sRGB;
        else
            coordinateSpace = CoordinateSpace::NO_PREFERENCE;

        return input;
    }

    inline std::ostream& operator<<(std::ostream& output, const CoordinateSpace& coordinateSpace)
    {
        if (coordinateSpace == CoordinateSpace::LINEAR)
            output << "LINEAR";
        else if (coordinateSpace == CoordinateSpace::sRGB)
            output << "sRGB";
        else
            output << "NO_PREFERENCE";

        return output;
    }

} // namespace vsg
