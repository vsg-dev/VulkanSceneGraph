#include <vsg/core/ref_ptr.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/Object.h>
#include <vsg/core/Auxiliary.h>
#include <vsg/core/Visitor.h>

#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>

#include <vsg/vk/Instance.h>

#include <iostream>
#include <vector>
#include <chrono>
#include <cstddef>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>


#include <osg/Matrixd>
#include <osg/io_utils>


namespace vsg
{
    // do we want to cover all the possible combinations?
    using ucvec2 = tvec2<unsigned char>;
    using cvec2 = tvec2<char>;

    using usvec2 = tvec2<unsigned short>;
    using svec2 = tvec2<short>;

    using ivec2 = tvec2<int>;
    using uivec2 = tvec2<unsigned int>;

    using ucvec3 = tvec3<unsigned char>;
    using cvec3 = tvec3<char>;

    using usvec3 = tvec3<unsigned short>;
    using svec3 = tvec3<short>;

    using ivec3 = tvec3<int>;
    using uivec3 = tvec3<unsigned int>;

    using ucvec4 = tvec4<unsigned char>;
    using cvec4 = tvec4<char>;

    using usvec4 = tvec4<unsigned short>;
    using svec4 = tvec4<short>;

    using ivec4 = tvec4<int>;
    using uivec4 = tvec4<unsigned int>;
}

std::unordered_map<std::type_index, VkFormat> VkFormatTypeMap;

template<typename T>
VkFormat VkFormatForType(T)
{
    return VkFormatTypeMap[std::type_index(typeid(T))];
}

template<typename T>
VkFormat VkFormatForType()
{
    return VkFormatTypeMap[std::type_index(typeid(T))];
}

int main(int /*argc*/, char** /*argv*/)
{
    std::cout<<"typeid(char) = "<<typeid(char).name()<<std::endl;
    std::cout<<"typeid(unsigned char) = "<<typeid(unsigned char).name()<<std::endl;
    std::cout<<"typeid(short) = "<<typeid(short).name()<<std::endl;
    std::cout<<"typeid(unsigned short) = "<<typeid(unsigned short).name()<<std::endl;
    std::cout<<"typeid(int) = "<<typeid(int).name()<<std::endl;
    std::cout<<"typeid(unsigned int) = "<<typeid(unsigned int).name()<<std::endl;
    std::cout<<"typeid(float) = "<<typeid(float).name()<<std::endl;
    std::cout<<"typeid(double) = "<<typeid(double).name()<<std::endl;
    std::cout<<"typeid(vsg::vec2) = "<<typeid(vsg::vec2).name()<<std::endl;
    std::cout<<"typeid(vsg::dvec2) = "<<typeid(vsg::dvec2).name()<<std::endl;
    std::cout<<"typeid(vsg::Vec3) = "<<typeid(vsg::vec3).name()<<std::endl;
    std::cout<<"typeid(vsg::Vec4) = "<<typeid(vsg::vec4).name()<<std::endl;



    VkFormatTypeMap[std::type_index(typeid(unsigned char))] =   VK_FORMAT_R8_UINT;
    VkFormatTypeMap[std::type_index(typeid(char))] =            VK_FORMAT_R8_SINT;
    VkFormatTypeMap[std::type_index(typeid(short))] =           VK_FORMAT_R16_SINT;
    VkFormatTypeMap[std::type_index(typeid(unsigned int))] =    VK_FORMAT_R32_UINT;
    VkFormatTypeMap[std::type_index(typeid(int))] =             VK_FORMAT_R32_SINT;
    VkFormatTypeMap[std::type_index(typeid(unsigned long))] =   VK_FORMAT_R64_UINT;
    VkFormatTypeMap[std::type_index(typeid(long))] =            VK_FORMAT_R64_SINT;
    VkFormatTypeMap[std::type_index(typeid(float))] =           VK_FORMAT_R32_SFLOAT;
    VkFormatTypeMap[std::type_index(typeid(double))] =          VK_FORMAT_R64_SFLOAT;

    VkFormatTypeMap[std::type_index(typeid(vsg::vec2))] =       VK_FORMAT_R32G32_SFLOAT;
    VkFormatTypeMap[std::type_index(typeid(vsg::dvec2))] =      VK_FORMAT_R64G64_SFLOAT;
    VkFormatTypeMap[std::type_index(typeid(vsg::uivec2))] =     VK_FORMAT_R32G32_UINT;
    VkFormatTypeMap[std::type_index(typeid(vsg::ivec2))] =      VK_FORMAT_R32G32_SINT;

    VkFormatTypeMap[std::type_index(typeid(vsg::vec3))] =       VK_FORMAT_R32G32B32_SFLOAT;
    VkFormatTypeMap[std::type_index(typeid(vsg::dvec3))] =      VK_FORMAT_R64G64B64_SFLOAT;
    VkFormatTypeMap[std::type_index(typeid(vsg::uivec3))] =     VK_FORMAT_R32G32B32_UINT;
    VkFormatTypeMap[std::type_index(typeid(vsg::ivec3))] =      VK_FORMAT_R32G32B32_SINT;

    VkFormatTypeMap[std::type_index(typeid(vsg::vec4))] =       VK_FORMAT_R32G32B32A32_SFLOAT;
    VkFormatTypeMap[std::type_index(typeid(vsg::dvec4))] =      VK_FORMAT_R64G64B64A64_SFLOAT;
    VkFormatTypeMap[std::type_index(typeid(vsg::uivec4))] =     VK_FORMAT_R32G32B32A32_UINT;
    VkFormatTypeMap[std::type_index(typeid(vsg::ivec4))] =      VK_FORMAT_R32G32B32A32_SINT;
    VkFormatTypeMap[std::type_index(typeid(vsg::ucvec4))] =     VK_FORMAT_R8G8B8A8_UINT;

    std::cout<<"VkFormat "<<VkFormatForType('c')<<std::endl;
    std::cout<<"VkFormat "<<VkFormatForType<unsigned char>()<<std::endl;
    std::cout<<"VkFormat "<<VkFormatForType<float>()<<std::endl;
    std::cout<<"VkFormat "<<VkFormatForType(1)<<std::endl;
    std::cout<<"VkFormat "<<VkFormatForType(1l)<<std::endl;
    std::cout<<"VkFormat "<<VkFormatForType(1.0f)<<std::endl;
    std::cout<<"VkFormat "<<VkFormatForType(1.0)<<std::endl;
    std::cout<<"VkFormat "<<VkFormatForType(vsg::vec2(1.0, 2.4))<<std::endl;
    std::cout<<"VkFormat "<<VkFormatForType(vsg::dvec2(1.0, 2.4))<<std::endl;
    std::cout<<"VkFormat "<<VkFormatForType(vsg::vec3(1.0, 2.4, 0.3))<<std::endl;
    std::cout<<"VkFormat "<<VkFormatForType(vsg::dvec3(1.0, 2.4, 0.2))<<std::endl;
    std::cout<<"VkFormat "<<VkFormatForType(vsg::vec4(1.0, 2.4, 0.1, 0.2))<<std::endl;
    std::cout<<"VkFormat "<<VkFormatForType(vsg::dvec4(1.0, 2.4, 0.1, 0.2))<<std::endl;
    std::cout<<"VkFormat "<<VkFormatForType(vsg::uivec4(1, 2, 0, 0))<<std::endl;
    std::cout<<"VkFormat "<<VkFormatForType(vsg::ucvec4(1, 2, 0, 0))<<std::endl;

    return 0;
}
