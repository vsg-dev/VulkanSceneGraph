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

template<typename T>
struct tvec2
{
    tvec2() : data{T(), T()} {}
    tvec2(T x, T y) : data{x, y} {}

    union
    {
        T data[2];
        struct { T x, y; };
        struct { T r, g; };
        struct { T s, t; };
    };

    T& operator[] (std::size_t i) { return data[i]; }
    T operator[] (std::size_t i) const { return data[i]; }


    // swizzle
    tvec2 yx() const { return tvec2(y,x); }
    tvec2 ts() const { return tvec2(t,s); }
    tvec2 gr() const { return tvec2(g,r); }
};

using vec2f = tvec2<float>;
using vec2d = tvec2<double>;
using vec2 = vec2f;

template<typename T>
inline std::ostream& operator << (std::ostream& output, const tvec2<T>& vec)
{
    output << vec.x << " " << vec.y;
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

    vec2d n(2.0, 1.0);
    std::cout<<"n(x="<<n.x<<", y="<<n.y<<")"<<std::endl;

    std::cout<<"n = "<<n<<std::endl;
    std::cout<<"n.yx() = "<<n.yx()<<std::endl;

    tvec2<int> i(2, 1);
    std::cout<<"i = "<<i<<std::endl;

    return 0;
}