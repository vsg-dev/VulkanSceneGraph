/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/ObjectFactory.h>

#include <vsg/core/Array.h>
#include <vsg/core/Value.h>

#include <vsg/nodes/Group.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>

#include <iostream>

using namespace vsg;

#define VSG_REGISTER_new(ClassName) _createMap[#ClassName] = []() { return ref_ptr<Object>(new ClassName()); }
#define VSG_REGISTER_create(ClassName) _createMap[#ClassName] = []() { return ClassName::create(); }

ObjectFactory::ObjectFactory()
{
    _createMap["nulltr"] = []() { return ref_ptr<Object>(); };

    VSG_REGISTER_new(vsg::Object);

    // values
    VSG_REGISTER_new(vsg::stringValue);
    VSG_REGISTER_new(vsg::boolValue);
    VSG_REGISTER_new(vsg::intValue);
    VSG_REGISTER_new(vsg::uintValue);
    VSG_REGISTER_new(vsg::floatValue);
    VSG_REGISTER_new(vsg::doubleValue);
    VSG_REGISTER_new(vsg::vec2Value);
    VSG_REGISTER_new(vsg::vec3Value);
    VSG_REGISTER_new(vsg::vec4Value);
    VSG_REGISTER_new(vsg::mat4Value);
    VSG_REGISTER_new(vsg::dvec2Value);
    VSG_REGISTER_new(vsg::dvec3Value);
    VSG_REGISTER_new(vsg::dvec4Value);
    VSG_REGISTER_new(vsg::dmat4Value);

    // ndodes
    VSG_REGISTER_create(vsg::Node);
    VSG_REGISTER_create(vsg::Group);
    VSG_REGISTER_create(vsg::QuadGroup);
    VSG_REGISTER_create(vsg::StateGroup);
}

vsg::ref_ptr<vsg::Object> ObjectFactory::create(const std::string& className)
{
    if (auto itr = _createMap.find(className); itr != _createMap.end())
    {
        //std::cout << "Using _createMap for " << className << std::endl;
        return (itr->second)();
    }

    //std::cout << "Warnig: ObjectFactory::create(" << className << ") failed to find means to create object" << std::endl;
    return vsg::ref_ptr<vsg::Object>();
}
