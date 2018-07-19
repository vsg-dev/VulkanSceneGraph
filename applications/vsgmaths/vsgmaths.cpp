#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/Object.h>
#include <vsg/core/Auxiliary.h>
#include <vsg/core/Visitor.h>

#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>
#include <vsg/maths/mat4.h>

#include <iostream>
#include <vector>
#include <chrono>
#include <cstddef>


#include <osg/Matrixd>
#include <osg/io_utils>

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

template<typename T>
inline std::ostream& operator << (std::ostream& output, const vsg::tmat4<T>& mat)
{
    output << mat(0,0)<< " " << mat(1,0)<<" "<<mat(2,0)<<" "<<mat(3,0)<<std::endl;
    output << mat(0,1)<< " " << mat(1,1)<<" "<<mat(2,1)<<" "<<mat(3,1)<<std::endl;
    output << mat(0,2)<< " " << mat(1,2)<<" "<<mat(2,2)<<" "<<mat(3,2)<<std::endl;
    output << mat(0,3)<< " " << mat(1,3)<<" "<<mat(2,3)<<" "<<mat(3,3)<<std::endl;
    return output; // to enable cascading
}

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
        using mat_type = tmat4<T>;

        vec_type forward = normalize(center-eye);
        vec_type up_normal = normalize(up);
        vec_type side = normalize(cross(forward, up_normal));
        vec_type u = normalize(cross(side, forward));

        return mat_type(side[0],     side[1],     side[2],      0,
                        u[0],        u[1],        u[2],         0,
                        -forward[0], -forward[1], -forward[2],  0,
                        0,           0,           0,            1);
    }

    const float PIf   = 3.14159265358979323846f;
    const double PI   = 3.14159265358979323846;

    float radians(float degrees) { return degrees * (PIf/180.0f); }
    double radians(double degrees) { return degrees * (PI/180.0); }

    float degrees(float radians) { return radians * (180.0f/PIf); }
    double degrees(double radians) { return radians * (180.0/PI); }
};

float computeDelta(const vsg::mat4& v, const osg::Matrixf& o)
{
    vsg::mat4::value_type delta = 0.0f;

    const vsg::mat4::value_type* view_ptr = v.data();
    const osg::Matrixf::value_type* osg_view_ptr = o.ptr();
    for(int i=0; i<16; ++i)
    {
        delta += abs(view_ptr[i]-osg_view_ptr[i]);
    }
    return delta;
}

float computeDelta(const vsg::dmat4& v, const osg::Matrixd& o)
{
    vsg::dmat4::value_type delta = 0.0f;

    const vsg::dmat4::value_type* view_ptr = v.data();
    const osg::Matrixd::value_type* osg_view_ptr = o.ptr();
    for(int i=0; i<16; ++i)
    {
        delta += abs(view_ptr[i]-osg_view_ptr[i]);
    }
    return delta;
}

int main(int argc, char** argv)
{

    vsg::vec2 v;

    v.x = 10.1f;
    v.y = 12.2f;


    std::cout<<"vec2(x="<<v.x<<", y="<<v.y<<")"<<std::endl;
    std::cout<<"vec2(r="<<v.r<<", g="<<v.g<<")"<<std::endl;
    std::cout<<"vec2(s="<<v.s<<", t="<<v.t<<")"<<std::endl;
    std::cout<<"vec2[0]=("<<v[0]<<", "<<v[1]<<")"<<std::endl;

    vsg::dvec3 n(2.0, 1.0, 0.5);
    std::cout<<"n(x="<<n.x<<", y="<<n.y<<", z="<<n.z<<")"<<std::endl;

    std::cout<<"n = "<<n<<std::endl;

    vsg::tvec2<int> i(2, 1);
    std::cout<<"i = "<<i<<std::endl;


    vsg::dvec4 colour(1.0, 0.9, 1.0, 0.5);
    std::cout<<"colour = ("<<colour<<")"<<std::endl;

    vsg::dmat4 mat;
    mat(3,0) = 102.3;
    std::cout<<"mat = "<<mat<<std::endl;

    vsg::tmat4<short> cmat;
    std::cout<<"cmat = "<<cmat<<std::endl;
    std::cout<<"sizeof(cmat) = "<<sizeof(cmat)<<std::endl;

    vsg::ref_ptr<vsg::Object> object = new vsg::Object;
    object->setValue("matrix", mat);

    vsg::mat4 new_mat;
    if (object->getValue("matrix", new_mat))
    {
        std::cout<<"getValue(\"matrix\""<<new_mat<<std::endl;
    }


    vsg::mat4 proj = vsg::perspective(vsg::radians(45.0), 2.0, 0.1, 10.0);

    std::cout<<"proj = {"<<std::endl<<proj<<"}"<<std::endl;

    osg::Matrixf osg_proj = osg::Matrixf::perspective(45.0f, 2.0f, 0.1f, 10.0f);
    std::cout<<"osg_proj = "<<osg_proj<<std::endl;

    std::cout<<std::endl;

    vsg::mat4 view = vsg::lookAt(vsg::vec3(0,0,0), vsg::vec3(0,10,0), vsg::vec3(0,0,1));

    std::cout<<"view = {"<<std::endl<<view<<"}"<<std::endl;

    osg::Matrixf osg_view = osg::Matrixf::lookAt(osg::Vec3(0,0,0), osg::Vec3(0,10, 0), osg::Vec3(0,0,1));
    std::cout<<"osg_view = "<<osg_view<<std::endl;


    float* view_ptr = view.data();
    float* osg_view_ptr = osg_view.ptr();
    for(int i=0; i<16; ++i)
    {
        std::cout<<" view_ptr["<<i<<"]="<<view_ptr[i]<<"   osg_voew_ptr["<<i<<"]="<<osg_view_ptr[i]<<std::endl;
    }

    std::cout<<"delta for proj "<<computeDelta(proj, osg_proj)<<std::endl;
    std::cout<<"delta for view "<<computeDelta(view, osg_view)<<std::endl;

    return 0;
}