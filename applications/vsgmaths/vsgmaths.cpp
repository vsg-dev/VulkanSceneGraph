#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/Object.h>
#include <vsg/core/Auxiliary.h>

#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>

#include <iostream>
#include <vector>
#include <chrono>


////////////////////////////////////////////////////////////////////
//
//  ostream implementation
//
template<typename T>
inline std::ostream& operator << (std::ostream& output, const vsg::tvec2<T>& vec)
{
    output << vec.x << " " << vec.y;
    return output; // to enable cascading
}

template<typename T>
inline std::ostream& operator << (std::ostream& output, const vsg::tvec3<T>& vec)
{
    output << vec.x << " " << vec.y<<" "<<vec.z;
    return output; // to enable cascading
}

template<typename T>
inline std::ostream& operator << (std::ostream& output, const vsg::tvec4<T>& vec)
{
    output << vec.x << " " << vec.y<<" "<<vec.z<<" "<<vec.w;
    return output; // to enable cascading
}

int main(int argc, char** argv)
{

    vsg::vec2 v;

    v.x = 10.1f;
    v.y = 12.2f;


    std::cout<<"vec2(x="<<v.x<<", y="<<v.y<<")"<<std::endl;
    std::cout<<"vec2(r="<<v.r<<", g="<<v.g<<")"<<std::endl;
    std::cout<<"vec2(s="<<v.s<<", t="<<v.t<<")"<<std::endl;
    std::cout<<"vec2.data="<<v.data<<" ("<<v.data[0]<<", "<<v.data[1]<<")"<<std::endl;
    std::cout<<"vec2[0]=("<<v[0]<<", "<<v[1]<<")"<<std::endl;

    vsg::vec3d n(2.0, 1.0, 0.5);
    std::cout<<"n(x="<<n.x<<", y="<<n.y<<", z="<<n.z<<")"<<std::endl;

    std::cout<<"n = "<<n<<std::endl;

    vsg::tvec2<int> i(2, 1);
    std::cout<<"i = "<<i<<std::endl;


    vsg::vec4d colour(1.0, 0.9, 1.0, 0.5);
    std::cout<<"colour = ("<<colour<<")"<<std::endl;


    return 0;
}