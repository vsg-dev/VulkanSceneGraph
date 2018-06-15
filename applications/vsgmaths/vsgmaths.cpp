#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/Object.h>
#include <vsg/core/Auxiliary.h>

#include <vsg/nodes/Group.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/LOD.h>

#include <iostream>
#include <vector>
#include <chrono>

////////////////////////////////////////////////////////////////////
//
//  vec2* implementation
//
template<typename T>
struct tvec2
{
    using value_type = T;

    union
    {
        value_type  data[2];
        struct { value_type x, y; };
        struct { value_type r, g; };
        struct { value_type s, t; };
    };

    tvec2() : data{value_type(), value_type()} {}
    tvec2(value_type x, value_type y) : data{x, y} {}

    std::size_t size() const { return 2; }

    value_type & operator[] (std::size_t i) { return data[i]; }
    value_type operator[] (std::size_t i) const { return data[i]; }
};

using vec2f = tvec2<float>;
using vec2d = tvec2<double>;
using vec2 = vec2f;

////////////////////////////////////////////////////////////////////
//
//  vec3* implementation
//
template<typename T>
struct tvec3
{
    using value_type = T;

    union
    {
        value_type  data[3];
        struct { value_type x, y, z; };
        struct { value_type r, g, b; };
        struct { value_type s, t, p; };
    };

    tvec3() : data{value_type(), value_type(), value_type()} {}
    tvec3(value_type x, value_type y, value_type z) : data{x, y, z} {}

    std::size_t size() const { return 3; }

    value_type & operator[] (std::size_t i) { return data[i]; }
    value_type operator[] (std::size_t i) const { return data[i]; }
};

using vec3f = tvec3<float>;
using vec3d = tvec3<double>;
using vec3 = vec3f;


////////////////////////////////////////////////////////////////////
//
//  vec4* implementation
//
template<typename T>
struct tvec4
{
    using value_type = T;

    union
    {
        value_type  data[4];
        struct { value_type x, y, z, w; };
        struct { value_type r, g, b, a; };
        struct { value_type s, t, p, q; };
    };

    tvec4() : data{value_type(), value_type(), value_type()} {}
    tvec4(value_type x, value_type y, value_type z, value_type w) : data{x, y, z, w} {}

    std::size_t size() const { return 4; }

    value_type & operator[] (std::size_t i) { return data[i]; }
    value_type operator[] (std::size_t i) const { return data[i]; }
};

using vec4f = tvec4<float>;
using vec4d = tvec4<double>;
using vec4 = vec4f;

////////////////////////////////////////////////////////////////////
//
//  ostream implementation
//
template<typename T>
inline std::ostream& operator << (std::ostream& output, const tvec2<T>& vec)
{
    output << vec.x << " " << vec.y;
    return output; // to enable cascading
}

template<typename T>
inline std::ostream& operator << (std::ostream& output, const tvec3<T>& vec)
{
    output << vec.x << " " << vec.y<<" "<<vec.z;
    return output; // to enable cascading
}

template<typename T>
inline std::ostream& operator << (std::ostream& output, const tvec4<T>& vec)
{
    output << vec.x << " " << vec.y<<" "<<vec.z<<" "<<vec.w;
    return output; // to enable cascading
}

int main(int argc, char** argv)
{

    vec2 v;

    v.x = 10.1f;
    v.y = 12.2f;


    std::cout<<"vec2(x="<<v.x<<", y="<<v.y<<")"<<std::endl;
    std::cout<<"vec2(r="<<v.r<<", g="<<v.g<<")"<<std::endl;
    std::cout<<"vec2(s="<<v.s<<", t="<<v.t<<")"<<std::endl;
    std::cout<<"vec2.data="<<v.data<<" ("<<v.data[0]<<", "<<v.data[1]<<")"<<std::endl;
    std::cout<<"vec2[0]=("<<v[0]<<", "<<v[1]<<")"<<std::endl;

    vec3d n(2.0, 1.0, 0.5);
    std::cout<<"n(x="<<n.x<<", y="<<n.y<<", z="<<n.z<<")"<<std::endl;

    std::cout<<"n = "<<n<<std::endl;

    tvec2<int> i(2, 1);
    std::cout<<"i = "<<i<<std::endl;


    vec4d colour(1.0, 0.9, 1.0, 0.5);
    std::cout<<"colour = ("<<colour<<")"<<std::endl;


    return 0;
}