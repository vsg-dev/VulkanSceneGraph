#include <vsg/core/Object.h>
#include <vsg/core/Auxiliary.h>
#include <vsg/core/Array.h>
#include <vsg/core/Visitor.h>

#include <iostream>
#include <algorithm>


template<typename T>
inline std::ostream& operator << (std::ostream& output, const vsg::tvec4<T>& vec)
{
    output << vec.x << " " << vec.y<<" "<<vec.z<<" "<<vec.w;
    return output; // to enable cascading
}

int main(int argc, char** argv)
{

    vsg::ref_ptr<vsg::floatArray> floats = new vsg::floatArray(10);

    std::cout<<"floats.size() = "<<floats->size()<<std::endl;

    float value = 0.0f;
    std::for_each(floats->begin(), floats->end(), [&value](float& v) {
        v = value++;
    });

    std::for_each(floats->begin(), floats->end(), [](float& v) {
        std::cout<<"   v[] = "<<v<<std::endl;
    });

    vsg::ref_ptr<vsg::vec4Array> colours = new vsg::vec4Array();
    colours->resize(20);
    vsg::vec4 c(0.25, 0.5, 0.75, 1.0);
    for(std::size_t i=0; i<colours->size(); ++i)
    {
        (*colours)[i] = c;
        c = vsg::vec4(c.g, c.b, c.a, c.r);
    }

    std::for_each(colours->begin(), colours->end(), [](vsg::vec4& c) {
        std::cout<<"   c[] = "<<c<<std::endl;
    });

    std::cout<<"colours->at(4) = "<<colours->at(4)<<std::endl;
    std::cout<<"(*colours)[5] = "<<(*colours)[5]<<std::endl;

    return 0;
}