/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/ObjectFactory.h>

#include <vsg/core/Array.h>
#include <vsg/core/Array2D.h>
#include <vsg/core/Array3D.h>
#include <vsg/core/Objects.h>
#include <vsg/core/Value.h>

#include <vsg/nodes/Commands.h>
#include <vsg/nodes/CullGroup.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexIndexDraw.h>

#include <vsg/vk/BindIndexBuffer.h>
#include <vsg/vk/BindVertexBuffers.h>
#include <vsg/vk/ComputePipeline.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/vk/DescriptorSet.h>
#include <vsg/vk/DescriptorSetLayout.h>
#include <vsg/vk/Draw.h>
#include <vsg/vk/GraphicsPipeline.h>
#include <vsg/vk/PipelineLayout.h>
#include <vsg/vk/ShaderModule.h>

#include <iostream>

using namespace vsg;

#define VSG_REGISTER_new(ClassName) _createMap[#ClassName] = []() { return ref_ptr<Object>(new ClassName()); }
#define VSG_REGISTER_create(ClassName) _createMap[#ClassName] = []() { return ClassName::create(); }

ObjectFactory::ObjectFactory()
{
    _createMap["nullptr"] = []() { return ref_ptr<Object>(); };

    VSG_REGISTER_new(vsg::Object);
    VSG_REGISTER_new(vsg::Objects);

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
    VSG_REGISTER_new(vsg::dvec2Value);
    VSG_REGISTER_new(vsg::dvec3Value);
    VSG_REGISTER_new(vsg::dvec4Value);
    VSG_REGISTER_new(vsg::ubvec2Value);
    VSG_REGISTER_new(vsg::ubvec3Value);
    VSG_REGISTER_new(vsg::ubvec4Value);
    VSG_REGISTER_new(vsg::usvec2Value);
    VSG_REGISTER_new(vsg::usvec3Value);
    VSG_REGISTER_new(vsg::usvec4Value);
    VSG_REGISTER_new(vsg::uivec2Value);
    VSG_REGISTER_new(vsg::uivec3Value);
    VSG_REGISTER_new(vsg::uivec4Value);
    VSG_REGISTER_new(vsg::mat4Value);
    VSG_REGISTER_new(vsg::dmat4Value);
    VSG_REGISTER_new(vsg::MaterialValue);

    // arrays
    VSG_REGISTER_new(vsg::ubyteArray);
    VSG_REGISTER_new(vsg::ushortArray);
    VSG_REGISTER_new(vsg::uintArray);
    VSG_REGISTER_new(vsg::floatArray);
    VSG_REGISTER_new(vsg::doubleArray);
    VSG_REGISTER_new(vsg::vec2Array);
    VSG_REGISTER_new(vsg::vec3Array);
    VSG_REGISTER_new(vsg::vec4Array);
    VSG_REGISTER_new(vsg::dvec2Array);
    VSG_REGISTER_new(vsg::dvec3Array);
    VSG_REGISTER_new(vsg::dvec4Array);
    VSG_REGISTER_new(vsg::ubvec2Array);
    VSG_REGISTER_new(vsg::ubvec3Array);
    VSG_REGISTER_new(vsg::ubvec4Array);
    VSG_REGISTER_new(vsg::usvec2Array);
    VSG_REGISTER_new(vsg::usvec3Array);
    VSG_REGISTER_new(vsg::usvec4Array);
    VSG_REGISTER_new(vsg::uivec2Array);
    VSG_REGISTER_new(vsg::uivec3Array);
    VSG_REGISTER_new(vsg::uivec4Array);
    VSG_REGISTER_new(vsg::mat4Array);
    VSG_REGISTER_new(vsg::dmat4Array);
    VSG_REGISTER_new(vsg::block64Array);
    VSG_REGISTER_new(vsg::block128Array);

    // array2Ds
    VSG_REGISTER_new(vsg::ubyteArray2D);
    VSG_REGISTER_new(vsg::ushortArray2D);
    VSG_REGISTER_new(vsg::uintArray2D);
    VSG_REGISTER_new(vsg::floatArray2D);
    VSG_REGISTER_new(vsg::doubleArray2D);
    VSG_REGISTER_new(vsg::vec2Array2D);
    VSG_REGISTER_new(vsg::vec3Array2D);
    VSG_REGISTER_new(vsg::vec4Array2D);
    VSG_REGISTER_new(vsg::dvec2Array2D);
    VSG_REGISTER_new(vsg::dvec3Array2D);
    VSG_REGISTER_new(vsg::dvec4Array2D);
    VSG_REGISTER_new(vsg::ubvec2Array2D);
    VSG_REGISTER_new(vsg::ubvec3Array2D);
    VSG_REGISTER_new(vsg::ubvec4Array2D);
    VSG_REGISTER_new(vsg::usvec2Array2D);
    VSG_REGISTER_new(vsg::usvec3Array2D);
    VSG_REGISTER_new(vsg::usvec4Array2D);
    VSG_REGISTER_new(vsg::uivec2Array2D);
    VSG_REGISTER_new(vsg::uivec3Array2D);
    VSG_REGISTER_new(vsg::uivec4Array2D);
    VSG_REGISTER_new(vsg::block64Array2D);
    VSG_REGISTER_new(vsg::block128Array2D);

    // array3Ds
    VSG_REGISTER_new(vsg::ubyteArray3D);
    VSG_REGISTER_new(vsg::ushortArray3D);
    VSG_REGISTER_new(vsg::uintArray3D);
    VSG_REGISTER_new(vsg::floatArray3D);
    VSG_REGISTER_new(vsg::doubleArray3D);
    VSG_REGISTER_new(vsg::vec2Array3D);
    VSG_REGISTER_new(vsg::vec3Array3D);
    VSG_REGISTER_new(vsg::vec4Array3D);
    VSG_REGISTER_new(vsg::dvec2Array3D);
    VSG_REGISTER_new(vsg::dvec3Array3D);
    VSG_REGISTER_new(vsg::dvec4Array3D);
    VSG_REGISTER_new(vsg::ubvec2Array3D);
    VSG_REGISTER_new(vsg::ubvec3Array3D);
    VSG_REGISTER_new(vsg::ubvec4Array3D);
    VSG_REGISTER_new(vsg::block64Array3D);
    VSG_REGISTER_new(vsg::block128Array3D);

    // nodes
    VSG_REGISTER_create(vsg::Node);
    VSG_REGISTER_create(vsg::Commands);
    VSG_REGISTER_create(vsg::Group);
    VSG_REGISTER_create(vsg::QuadGroup);
    VSG_REGISTER_create(vsg::StateGroup);
    VSG_REGISTER_create(vsg::CullGroup);
    VSG_REGISTER_create(vsg::CullNode);
    VSG_REGISTER_create(vsg::MatrixTransform);
    VSG_REGISTER_create(vsg::Geometry);
    VSG_REGISTER_create(vsg::VertexIndexDraw);

    // vulkan objects
    VSG_REGISTER_create(vsg::BindGraphicsPipeline);
    VSG_REGISTER_create(vsg::PipelineLayout);
    VSG_REGISTER_create(vsg::GraphicsPipeline);
    VSG_REGISTER_create(vsg::BindComputePipeline);
    VSG_REGISTER_create(vsg::ComputePipeline);
    VSG_REGISTER_create(vsg::ShaderStages);
    VSG_REGISTER_create(vsg::ShaderModule);
    VSG_REGISTER_create(vsg::Texture);
    VSG_REGISTER_create(vsg::Uniform);
    VSG_REGISTER_create(vsg::VertexInputState);
    VSG_REGISTER_create(vsg::InputAssemblyState);
    VSG_REGISTER_create(vsg::RasterizationState);
    VSG_REGISTER_create(vsg::MultisampleState);
    VSG_REGISTER_create(vsg::ColorBlendState);
    VSG_REGISTER_create(vsg::ViewportState);
    VSG_REGISTER_create(vsg::MultisampleState);
    VSG_REGISTER_create(vsg::DepthStencilState);
    VSG_REGISTER_create(vsg::ColorBlendState);
    VSG_REGISTER_create(vsg::Draw);
    VSG_REGISTER_create(vsg::DrawIndexed);
    VSG_REGISTER_create(vsg::BindDescriptorSets);
    VSG_REGISTER_create(vsg::BindDescriptorSet);
    VSG_REGISTER_create(vsg::BindVertexBuffers);
    VSG_REGISTER_create(vsg::BindIndexBuffer);
    VSG_REGISTER_create(vsg::DescriptorSet);
    VSG_REGISTER_create(vsg::DescriptorSetLayout);
    VSG_REGISTER_create(vsg::Texture);
    VSG_REGISTER_create(vsg::Uniform);
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
