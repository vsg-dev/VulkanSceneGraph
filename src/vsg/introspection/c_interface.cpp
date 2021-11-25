/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/introspection/c_interface.h>

#include <vsg/core/Object.h>
#include <vsg/core/Visitor.h>
#include <vsg/io/Options.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>

#include <functional>
#include <iostream>
#include <map>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace vsg
{

    class TypeDescriptor : public vsg::Object
    {
    public:
        using Properties = std::map<std::string, ref_ptr<TypeDescriptor>>;
        using CreateFunction = std::function<Object*()>;

        std::string className;
        CreateFunction createFunction;
        Properties properties;
    };

    class Introspection : public Object
    {
    public:
        Introspection()
        {

            std::cout << "Introspection()" << std::endl;

            add(std::type_index(typeid(vsg::Group)), vsg::type_name<vsg::Group>(), []() { return new vsg::Group; });
            add(std::type_index(typeid(vsg::LOD)), vsg::type_name<vsg::LOD>(), []() { return new vsg::LOD; });
            add(std::type_index(typeid(vsg::Node)), vsg::type_name<vsg::Node>(), []() { return new vsg::Node; });
        }
        virtual ~Introspection() { std::cout << "~Introspection()" << std::endl; }

        using IndexTypeDescriptorMap = std::unordered_map<std::type_index, ref_ptr<TypeDescriptor>>;
        using NameTypeDescriptorMap = std::unordered_map<std::string, ref_ptr<TypeDescriptor>>;

        IndexTypeDescriptorMap _indexTypeDescriptorMap;
        NameTypeDescriptorMap _nameTypeDescriptorMap;

        void add(std::type_index typeindex, const char* className, TypeDescriptor::CreateFunction createFunction)
        {
            TypeDescriptor* td = new TypeDescriptor;
            td->className = className;
            td->createFunction = createFunction;

            _indexTypeDescriptorMap[typeindex] = td;
            _nameTypeDescriptorMap[className] = td;

            std::cout << "    add(..., " << className << ", ,,)" << std::endl;
        }

        static ref_ptr<Introspection>& instance()
        {
            static ref_ptr<Introspection> s_introspection(new Introspection);
            return s_introspection;
        }

        vsg::Object* create(const char* className) const
        {
            std::cout << "create(" << className << ")" << std::endl;
            NameTypeDescriptorMap::const_iterator itr = _nameTypeDescriptorMap.find(className);
            if (itr != _nameTypeDescriptorMap.end())
            {
                std::cout << "  found TypeDescriptor()" << std::endl;
                return (itr->second->createFunction)();
            }
            else
            {
                std::cout << "  could not find TypeDescriptor()" << std::endl;
                return nullptr;
            }
        }

        TypeDescriptor* typeDescriptor(const vsg::Object* object) const
        {
            IndexTypeDescriptorMap::const_iterator itr = _indexTypeDescriptorMap.find(std::type_index(typeid(*object)));
            return (itr != _indexTypeDescriptorMap.end()) ? itr->second.get() : nullptr;
        }
    };

} // namespace vsg

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
        if (object)
        {
            return reinterpret_cast<vsg::Object*>(object)->className();
        }
        else
        {
            return 0;
        }
    }

    vsgObjectPtr vsgMethod(vsgObjectPtr /*object*/, const char* /*methodName*/)
    {
        return 0;
    }

    class ObjectToPropertyVisitor : public vsg::Visitor
    {
    public:
        Property _property = {};

        void apply(vsg::Object& value) override
        {
            _property.type = Property::TYPE_Object;
            _property.value._object = &value;
            std::cout << "apply(vsg::Object&)" << std::endl;
        }
        void apply(vsg::boolValue& value) override
        {
            _property.type = Property::TYPE_bool;
            _property.value._bool = value.value();
            std::cout << "apply(vsg::boolValue&)" << std::endl;
        }
        void apply(vsg::intValue& value) override
        {
            _property.type = Property::TYPE_int;
            _property.value._int = value.value();
            std::cout << "apply(vsg::intValue&)" << std::endl;
        }
    };

    struct Property vsgGetProperty(vsgObjectPtr objectPtr, const char* propertyName)
    {
        if (!objectPtr) return Property{Property::TYPE_undefined, {0}};

        vsg::Object* object = reinterpret_cast<vsg::Object*>(objectPtr);
        vsg::Object* propertyObject = object->getObject(propertyName);
        if (propertyObject)
        {
            ObjectToPropertyVisitor otpv;
            propertyObject->accept(otpv);

            std::cout << "Return object" << std::endl;
            return otpv._property;
        }
        else
        {
            std::cout << "Return empty, fallback to TYPE_undefined." << std::endl;

            Property property;
            property.type = Property::TYPE_undefined;
            return property;
        }
    }

    void vsgSetProperty(vsgObjectPtr objectPtr, const char* propertyName, struct Property property)
    {
        std::cout << "vsgSetProperty(" << objectPtr << ", " << propertyName << ", " << property.type << std::endl;
        if (!objectPtr) return;
        vsg::Object* object = reinterpret_cast<vsg::Object*>(objectPtr);
#if 1
        switch (property.type)
        {
        case (Property::TYPE_Object): object->setObject(propertyName, reinterpret_cast<vsg::Object*>(property.value._object)); break;
        case (Property::TYPE_bool): object->setValue(propertyName, bool(property.value._bool != 0)); break;
        case (Property::TYPE_char): object->setValue(propertyName, property.value._char); break;
        case (Property::TYPE_unsigned_char): object->setValue(propertyName, property.value._unsigned_char); break;
        case (Property::TYPE_short): object->setValue(propertyName, property.value._short); break;
        case (Property::TYPE_unsigned_short): object->setValue(propertyName, property.value._unsigned_short); break;
        case (Property::TYPE_int): object->setValue(propertyName, property.value._int); break;
        case (Property::TYPE_unsigned_int): object->setValue(propertyName, property.value._unsigned_int); break;
        case (Property::TYPE_float): object->setValue(propertyName, property.value._float); break;
        case (Property::TYPE_double): object->setValue(propertyName, property.value._double); break;
        default: std::cout << "Unhandled Property type" << std::endl;
        }
#endif
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
