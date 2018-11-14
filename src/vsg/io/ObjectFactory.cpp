/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/ObjectFactory.h>

#include <vsg/nodes/Group.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>

#include <iostream>

using namespace vsg;

ObjectFactory::ObjectFactory()
{
    _createMap["nulltr"] = []() { return vsg::ref_ptr<vsg::Object>(); };
    _createMap["vsg::Object"] = []() { return vsg::ref_ptr<vsg::Object>(new vsg::Object()); };

    // ndodes
    _createMap["vsg::Node"] = []() { return vsg::Node::create(); };
    _createMap["vsg::Group"] = []() { return vsg::Group::create(); };
    _createMap["vsg::QuadGroup"] = []() { return vsg::QuadGroup::create(); };
    _createMap["vsg::StateGroup"] = []() { return vsg::StateGroup::create(); };
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
