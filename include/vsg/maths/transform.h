#pragma once

#include <vsg/maths/vec3.h>
#include <vsg/maths/mat4.h>

namespace vsg
{
    template<typename T>
    tmat4<T> perspective(T fovy_radians, T aspectRatio, T zNear, T zFar)
    {
        T f = 1.0/tan(fovy_radians*0.5);
        T r = 1.0/(zNear-zFar);
        return tmat4<T>(f/aspectRatio, 0,  0,              0,
                        0,             -f, 0,              0,
                        0,             0,  (zFar+zNear)*r, (2.0*zFar*zNear)*r,
                        0,             0,  -1,             0);
    }

    template<typename T>
    tmat4<T> lookAt(tvec3<T> const & eye, tvec3<T> const & center, tvec3<T> const & up )
    {
        using vec_type = tvec3<T>;

        vec_type forward = normalize(center-eye);
        vec_type up_normal = normalize(up);
        vec_type side = normalize(cross(forward, up_normal));
        vec_type u = normalize(cross(side, forward));

        return tmat4<T>(side[0],     side[1],     side[2],      0,
                        u[0],        u[1],        u[2],         0,
                        -forward[0], -forward[1], -forward[2],  0,
                        0,           0,           0,            1);
    }

    template<typename T>
    tmat4<T> rotate(T angle_radians, T x, T y, T z)
    {
        const T c = cos(angle_radians);
        const T s = sin(angle_radians);
        const T one_minus_c = 1-c;
        return tmat4<T>(x*x*one_minus_c+c,     x*y*one_minus_c-z*s, x*z*one_minus_c+y*z, 0,
                        y*x*one_minus_c+z*s,   y*y*one_minus_c+c,   y*z*one_minus_c-x*s, 0,
                        x*z*one_minus_c-y*s,   y*z*one_minus_c+x*s, z*z*one_minus_c+c,   0,
                        0,                     0,                   0,                   1);
    }

    template<typename T>
    tmat4<T> rotate(T angle_radians, const tvec3<T>& v)
    {
        return (angle_radians, v.x, v.y, v.z);
    }

    template<typename T>
    tmat4<T> translate(T x, T y, T z)
    {
        return tmat4<T>(1, 0, 0, x,
                        0, 1, 0, y,
                        0, 0, 1, z,
                        0, 0, 0, 1);
    }

    template<typename T>
    tmat4<T> translate(const tvec3<T>& v)
    {
        return translate(v.x, v.y, v.z);
    }

    template<typename T>
    tmat4<T> scale(T sx, T sy, T sz)
    {
        return tmat4<T>(sx, 0,  0,  0,
                        0,  sy, 0,  0,
                        0,  0,  sz, 0,
                        0,  0,  0,  1);
    }

    template<typename T>
    tmat4<T> scale(const tvec3<T>& v)
    {
        return scale(v.x, v.y, v.z);
    }

}
