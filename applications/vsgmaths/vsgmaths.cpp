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


struct vec2
{
    vec2() : data{0.0, 0.0} {}
    vec2(float x, float y) : data{x, y} {}

    union
    {
        float data[2];
        struct { float x, y; };
        struct { float r, g; };
        struct { float s, t; };
    };

    float& operator[] (std::size_t i) { return data[i]; }
    float operator[] (std::size_t i) const { return data[i]; }


    // swizzle
    vec2 yx() const { return vec2(y,x); }
    vec2 ts() const { return vec2(t,s); }
    vec2 gr() const { return vec2(g,r); }
};

inline std::ostream& operator << (std::ostream& output, const vec2& vec)
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

    vec2 n(2.0f, 1.0f);
    std::cout<<"n(x="<<n.x<<", y="<<n.y<<")"<<std::endl;

    std::cout<<"n = "<<n<<std::endl;
    std::cout<<"n.yx() = "<<n.yx()<<std::endl;

    return 0;
}