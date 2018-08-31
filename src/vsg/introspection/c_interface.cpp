#include <vsg/introspection/c_interface.h>


#include <vsg/core/Object.h>
#include <vsg/nodes/Group.h>

#include <typeinfo>
#include <typeindex>
#include <map>
#include <unordered_map>
#include <iostream>

namespace vsg
{


class TypeDescriptor : public vsg::Object
{
public:
    using Properties = std::map<std::string, ref_ptr<TypeDescriptor>>;

    std::string className;
    Properties  properties;
};

class Introspection : public Object
{
public:

    Introspection() { std::cout<<"Introspection()"<<std::endl; }
    virtual ~Introspection() { std::cout<<"~Introspection()"<<std::endl; }

    using IndexTypeDescriptorMap = std::unordered_map<std::type_index, ref_ptr<TypeDescriptor>>;
    using NameTypeDescriptorMap = std::unordered_map<std::string, ref_ptr<TypeDescriptor>>;

    IndexTypeDescriptorMap  _indexTypeDescriptorMap;
    NameTypeDescriptorMap   _nameTypeDescriptorMap;

    static ref_ptr<Introspection>& instance()
    {
        static ref_ptr<Introspection> s_introspection = new Introspection;
        return s_introspection;
    }

    vsg::Object* create(const char* /*className*/)
    {
        return new vsg::Group;
    }
};

}


extern "C"
{

vsgObjectPtr vsgCreate(const char* className)
{
    return vsg::Introspection::instance()->create(className);
}

vsgObjectPtr vsgMethod(vsgObjectPtr /*object*/, const char* /*methodName*/)
{
    return 0;
}

vsgObjectPtr vsgGetProperty(vsgObjectPtr /*object*/, const char* /*propertyName*/)
{
    return 0;
}

vsgObjectPtr vsgSetProperty(vsgObjectPtr /*object*/, const char* /*propertyName*/, vsgObjectPtr* /*value*/)
{
    return 0;
}

void vsgRef(vsgObjectPtr object)
{
    if (object)
    {
        reinterpret_cast<vsg::Object*>(object)->ref();
    }
}

void vsgUnref(vsgObjectPtr object)
{
    if (object)
    {
        reinterpret_cast<vsg::Object*>(object)->unref();
    }
}

unsigned int vsgGetNumProperties(vsgObjectPtr /*object*/)
{
    return 0;
}

const char* vsgGetPropertyName(vsgObjectPtr /*object*/, unsigned int /*index*/)
{
    return 0;
}

}
