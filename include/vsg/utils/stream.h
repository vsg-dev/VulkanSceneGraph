#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>
#include <vsg/maths/mat4.h>

#include <ostream>
#include <sstream>

namespace vsg
{
    template<typename T>
    inline std::ostream& operator << (std::ostream& output, const vsg::t_vec2<T>& vec)
    {
        output << vec.x << " " << vec.y;
        return output; // to enable cascading
    }

    template<typename T>
    inline std::ostream& operator << (std::ostream& output, const vsg::t_vec3<T>& vec)
    {
        output << vec.x << " " << vec.y<<" "<<vec.z;
        return output; // to enable cascading
    }

    template<typename T>
    inline std::ostream& operator << (std::ostream& output, const vsg::t_vec4<T>& vec)
    {
        output << vec.x << " " << vec.y<<" "<<vec.z<<" "<<vec.w;
        return output; // to enable cascading
    }

    template<typename T>
    inline std::ostream& operator << (std::ostream& output, const vsg::t_mat4<T>& mat)
    {
        output << std::endl;
        output << "    "<<mat(0,0)<< " " << mat(1,0)<<" "<<mat(2,0)<<" "<<mat(3,0)<<std::endl;
        output << "    "<<mat(0,1)<< " " << mat(1,1)<<" "<<mat(2,1)<<" "<<mat(3,1)<<std::endl;
        output << "    "<<mat(0,2)<< " " << mat(1,2)<<" "<<mat(2,2)<<" "<<mat(3,2)<<std::endl;
        output << "    "<<mat(0,3)<< " " << mat(1,3)<<" "<<mat(2,3)<<" "<<mat(3,3)<<std::endl;
        return output; // to enable cascading
    }

    template< typename ... Args >
    std::string make_string(Args const& ... args )
    {
        std::ostringstream stream;
        (stream<< ...<<args);
        return stream.str();
    }
}


