#include <vsg/introspection/c_interface.h>


#include <vsg/core/Object.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>

#include <typeinfo>
#include <typeindex>
#include <map>
#include <unordered_map>
#include <iostream>
#include <functional>

namespace vsg
{


class TypeDescriptor : public vsg::Object
{
public:
    using Properties = std::map<std::string, ref_ptr<TypeDescriptor>>;
    using CreateFunction = std::function<Object*()>;

    std::string     className;
    CreateFunction  createFunction;
    Properties  properties;
};

class Introspection : public Object
{
public:

    Introspection() {

        std::cout<<"Introspection()"<<std::endl;

        add(std::type_index(typeid(vsg::Group)), "vsg::Group", [](){ return new vsg::Group; });
        add(std::type_index(typeid(vsg::LOD)), "vsg::LOD", [](){ return new vsg::LOD; });
        add(std::type_index(typeid(vsg::Node)), "vsg::Node", [](){ return new vsg::Node; });

    }
    virtual ~Introspection() { std::cout<<"~Introspection()"<<std::endl; }

    using IndexTypeDescriptorMap = std::unordered_map<std::type_index, ref_ptr<TypeDescriptor>>;
    using NameTypeDescriptorMap = std::unordered_map<std::string, ref_ptr<TypeDescriptor>>;

    IndexTypeDescriptorMap  _indexTypeDescriptorMap;
    NameTypeDescriptorMap   _nameTypeDescriptorMap;

    void add(std::type_index typeindex, const char* className, TypeDescriptor::CreateFunction createFunction)
    {
        TypeDescriptor* typeDescriptor = new TypeDescriptor;
        typeDescriptor->className = className;
        typeDescriptor->createFunction = createFunction;

        _indexTypeDescriptorMap[typeindex] = typeDescriptor;
        _nameTypeDescriptorMap[className] = typeDescriptor;

        std::cout<<"    add(..., "<<className<<", ,,)"<<std::endl;
    }

    static ref_ptr<Introspection>& instance()
    {
        static ref_ptr<Introspection> s_introspection = new Introspection;
        return s_introspection;
    }

    vsg::Object* create(const char* className) const
    {
        std::cout<<"create("<<className<<")"<<std::endl;
        NameTypeDescriptorMap::const_iterator itr = _nameTypeDescriptorMap.find(className);
        if (itr != _nameTypeDescriptorMap.end())
        {
            std::cout<<"  found TypeDescriptor()"<<std::endl;
            return (itr->second->createFunction)();
        }
        else
        {
            std::cout<<"  could not find TypeDescriptor()"<<std::endl;
            return nullptr;
        }
    }

    TypeDescriptor* typeDescriptor(const vsg::Object* object) const
    {
        IndexTypeDescriptorMap::const_iterator itr = _indexTypeDescriptorMap.find(std::type_index(typeid(*object)));
        return (itr != _indexTypeDescriptorMap.end()) ? itr->second : nullptr;
    }

};

}


extern "C"
{

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

vsgObjectPtr vsgCreate(const char* className)
{
    return vsg::Introspection::instance()->create(className);
}

const char* vsgClassName(vsgObjectPtr object)
{
    vsg::TypeDescriptor* td = vsg::Introspection::instance()->typeDescriptor(reinterpret_cast<vsg::Object*>(object));
    if (td) return td->className.c_str();
    else return 0;
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

unsigned int vsgGetNumProperties(vsgObjectPtr /*object*/)
{
    return 0;
}

const char* vsgGetPropertyName(vsgObjectPtr /*object*/, unsigned int /*index*/)
{
    return 0;
}

}
