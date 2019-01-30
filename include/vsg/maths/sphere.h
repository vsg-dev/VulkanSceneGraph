#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

namespace vsg
{
    template<typename T>
    struct t_sphere
    {
        using value_type = T;
        using vec_type = t_vec3<T>;

        t_sphere() :
            center{0.0, 0.0, 0.0},
            radius(-1.0) {}

        t_sphere(const vec_type& c, value_type r) :
            center(c),
            radius(r) {}

        bool valid() const { return radius >= 0.0; }

        vec_type center;
        value_type radius;
    };

    using sphere = t_sphere<float>;
    using dsphere = t_sphere<double>;

    VSG_type_name(vsg::sphere);
    VSG_type_name(vsg::dsphere);
} // namespace vsg
