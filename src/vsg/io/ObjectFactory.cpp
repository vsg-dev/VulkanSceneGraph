/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/all.h>

using namespace vsg;

ref_ptr<ObjectFactory>& ObjectFactory::instance()
{
    // declare the ObjectFactory singleton as static to be initialized on first invocation of the instance() method.  Note, this currently assumes that initialization won't be multi-threaded.
    static ref_ptr<ObjectFactory> s_ObjectFactory(new ObjectFactory);
    return s_ObjectFactory;
}

ObjectFactory::ObjectFactory()
{
    _createMap["nullptr"] = []() { return ref_ptr<Object>(); };

    // cores
    add<vsg::Object>();
    add<vsg::Objects>();
    add<vsg::External>();

    // values
    add<vsg::stringValue>();
    add<vsg::boolValue>();
    add<vsg::intValue>();
    add<vsg::uintValue>();
    add<vsg::floatValue>();
    add<vsg::doubleValue>();
    add<vsg::vec2Value>();
    add<vsg::vec3Value>();
    add<vsg::vec4Value>();
    add<vsg::dvec2Value>();
    add<vsg::dvec3Value>();
    add<vsg::dvec4Value>();
    add<vsg::ubvec2Value>();
    add<vsg::ubvec3Value>();
    add<vsg::ubvec4Value>();
    add<vsg::usvec2Value>();
    add<vsg::usvec3Value>();
    add<vsg::usvec4Value>();
    add<vsg::uivec2Value>();
    add<vsg::uivec3Value>();
    add<vsg::uivec4Value>();
    add<vsg::mat4Value>();
    add<vsg::dmat4Value>();
    add<vsg::materialValue>();
    add<vsg::PhongMaterialValue>();
    add<vsg::PbrMaterialValue>();
    add<vsg::sphereValue>();
    add<vsg::boxValue>();
    add<vsg::quatValue>();
    add<vsg::dsphereValue>();
    add<vsg::dboxValue>();
    add<vsg::dquatValue>();

    // arrays
    add<vsg::byteArray>();
    add<vsg::ubyteArray>();
    add<vsg::shortArray>();
    add<vsg::ushortArray>();
    add<vsg::intArray>();
    add<vsg::uintArray>();
    add<vsg::floatArray>();
    add<vsg::doubleArray>();
    add<vsg::vec2Array>();
    add<vsg::vec3Array>();
    add<vsg::vec4Array>();
    add<vsg::dvec2Array>();
    add<vsg::dvec3Array>();
    add<vsg::dvec4Array>();
    add<vsg::bvec2Array>();
    add<vsg::bvec3Array>();
    add<vsg::bvec4Array>();
    add<vsg::ubvec2Array>();
    add<vsg::ubvec3Array>();
    add<vsg::ubvec4Array>();
    add<vsg::svec2Array>();
    add<vsg::svec3Array>();
    add<vsg::svec4Array>();
    add<vsg::usvec2Array>();
    add<vsg::usvec3Array>();
    add<vsg::usvec4Array>();
    add<vsg::ivec2Array>();
    add<vsg::ivec3Array>();
    add<vsg::ivec4Array>();
    add<vsg::uivec2Array>();
    add<vsg::uivec3Array>();
    add<vsg::uivec4Array>();
    add<vsg::mat4Array>();
    add<vsg::dmat4Array>();
    add<vsg::block64Array>();
    add<vsg::block128Array>();
    add<vsg::materialArray>();
    add<vsg::PhongMaterialArray>();
    add<vsg::PbrMaterialArray>();
    add<vsg::DrawIndirectCommandArray>();

    // array2Ds
    add<vsg::byteArray2D>();
    add<vsg::ubyteArray2D>();
    add<vsg::shortArray2D>();
    add<vsg::ushortArray2D>();
    add<vsg::intArray2D>();
    add<vsg::uintArray2D>();
    add<vsg::floatArray2D>();
    add<vsg::doubleArray2D>();
    add<vsg::vec2Array2D>();
    add<vsg::vec3Array2D>();
    add<vsg::vec4Array2D>();
    add<vsg::dvec2Array2D>();
    add<vsg::dvec3Array2D>();
    add<vsg::dvec4Array2D>();
    add<vsg::bvec2Array2D>();
    add<vsg::bvec3Array2D>();
    add<vsg::bvec4Array2D>();
    add<vsg::ubvec2Array2D>();
    add<vsg::ubvec3Array2D>();
    add<vsg::ubvec4Array2D>();
    add<vsg::svec2Array2D>();
    add<vsg::svec3Array2D>();
    add<vsg::svec4Array2D>();
    add<vsg::usvec2Array2D>();
    add<vsg::usvec3Array2D>();
    add<vsg::usvec4Array2D>();
    add<vsg::ivec2Array2D>();
    add<vsg::ivec3Array2D>();
    add<vsg::ivec4Array2D>();
    add<vsg::uivec2Array2D>();
    add<vsg::uivec3Array2D>();
    add<vsg::uivec4Array2D>();
    add<vsg::block64Array2D>();
    add<vsg::block128Array2D>();

    // array3Ds
    add<vsg::byteArray3D>();
    add<vsg::ubyteArray3D>();
    add<vsg::shortArray3D>();
    add<vsg::ushortArray3D>();
    add<vsg::intArray3D>();
    add<vsg::uintArray3D>();
    add<vsg::floatArray3D>();
    add<vsg::doubleArray3D>();
    add<vsg::vec2Array3D>();
    add<vsg::vec3Array3D>();
    add<vsg::vec4Array3D>();
    add<vsg::dvec2Array3D>();
    add<vsg::dvec3Array3D>();
    add<vsg::dvec4Array3D>();
    add<vsg::ubvec2Array3D>();
    add<vsg::ubvec3Array3D>();
    add<vsg::ubvec4Array3D>();
    add<vsg::block64Array3D>();
    add<vsg::block128Array3D>();

    // nodes
    add<vsg::Node>();
    add<vsg::Commands>();
    add<vsg::Group>();
    add<vsg::QuadGroup>();
    add<vsg::StateGroup>();
    add<vsg::CullGroup>();
    add<vsg::CullNode>();
    add<vsg::LOD>();
    add<vsg::PagedLOD>();
    add<vsg::AbsoluteTransform>();
    add<vsg::MatrixTransform>();
    add<vsg::Geometry>();
    add<vsg::VertexDraw>();
    add<vsg::VertexIndexDraw>();
    add<vsg::Bin>();
    add<vsg::DepthSorted>();
    add<vsg::Switch>();
    add<vsg::Light>();
    add<vsg::AmbientLight>();
    add<vsg::DirectionalLight>();
    add<vsg::PointLight>();
    add<vsg::SpotLight>();
    add<vsg::TileDatabase>();
    add<vsg::TileDatabaseSettings>();

    // vulkan objects
    add<vsg::BindGraphicsPipeline>();
    add<vsg::PipelineLayout>();
    add<vsg::GraphicsPipeline>();
    add<vsg::BindComputePipeline>();
    add<vsg::ComputePipeline>();
    add<vsg::ShaderStage>();
    add<vsg::ShaderModule>();
    add<vsg::ShaderCompileSettings>();
    add<vsg::VertexInputState>();
    add<vsg::InputAssemblyState>();
    add<vsg::TessellationState>();
    add<vsg::RasterizationState>();
    add<vsg::MultisampleState>();
    add<vsg::ColorBlendState>();
    add<vsg::ViewportState>();
    add<vsg::MultisampleState>();
    add<vsg::DepthStencilState>();
    add<vsg::ColorBlendState>();
    add<vsg::DynamicState>();
    add<vsg::Dispatch>();
    add<vsg::BindDescriptorSets>();
    add<vsg::BindDescriptorSet>();
    add<vsg::BindVertexBuffers>();
    add<vsg::BindIndexBuffer>();
    add<vsg::BindViewDescriptorSets>();
    add<vsg::DescriptorSet>();
    add<vsg::DescriptorSetLayout>();
    add<vsg::ViewDescriptorSetLayout>();
    add<vsg::DescriptorImage>();
    add<vsg::DescriptorBuffer>();
    add<vsg::Sampler>();
    add<vsg::PushConstants>();
    add<vsg::ResourceHints>();
    add<vsg::StateSwitch>();

    // commands
    add<vsg::Draw>();
    add<vsg::DrawIndirect>();
    add<vsg::DrawIndexed>();
    add<vsg::DrawIndexedIndirect>();
    add<vsg::CopyImage>();
    add<vsg::BlitImage>();
    add<vsg::QueryPool>();
    add<vsg::WriteTimestamp>();
    add<vsg::BeginQuery>();
    add<vsg::EndQuery>();
    add<vsg::ResetQueryPool>();
    add<vsg::CopyQueryPoolResults>();

    // text
    add<vsg::GlyphMetricsArray>();
    add<vsg::Font>();
    add<vsg::Text>();
    add<vsg::TextGroup>();
    add<vsg::StandardLayout>();
    add<vsg::CpuLayoutTechnique>();
    add<vsg::GpuLayoutTechnique>();
    add<vsg::TextLayoutValue>();

    // ui
    add<vsg::UIEvent>();
    add<vsg::TerminateEvent>();
    add<vsg::FrameStamp>();
    add<vsg::FrameEvent>();
    add<vsg::PointerEvent>();
    add<vsg::ButtonPressEvent>();
    add<vsg::ButtonReleaseEvent>();
    add<vsg::MoveEvent>();
    add<vsg::TouchEvent>();
    add<vsg::TouchDownEvent>();
    add<vsg::TouchUpEvent>();
    add<vsg::TouchMoveEvent>();
    add<vsg::ScrollWheelEvent>();
    add<vsg::WindowEvent>();
    add<vsg::ExposeWindowEvent>();
    add<vsg::ConfigureWindowEvent>();
    add<vsg::CloseWindowEvent>();
    add<vsg::KeyEvent>();
    add<vsg::KeyPressEvent>();
    add<vsg::KeyReleaseEvent>();

    // viewer
    add<vsg::Camera>();
    add<vsg::LookAt>();
    add<vsg::Perspective>();

    // mesh shading
    add<vsg::DrawMeshTasks>();
    add<vsg::DrawMeshTasksIndirect>();
    add<vsg::DrawMeshTasksIndirectCount>();

    // io
    add<vsg::Options>();
    add<vsg::CompositeReaderWriter>();
    add<vsg::VSG>();
    add<vsg::spirv>();
    add<vsg::ArrayState>();
    add<vsg::NullArrayState>();

    // utils
    add<vsg::AnimationPath>();
    add<vsg::ShaderSet>();
    add<vsg::PositionAndDisplacementMapArrayState>();
    add<vsg::DisplacementMapArrayState>();
    add<vsg::PositionArrayState>();
    add<vsg::BillboardArrayState>();
    add<vsg::SharedObjects>();

    // application
    add<vsg::EllipsoidModel>();
}

ObjectFactory::~ObjectFactory()
{
}

vsg::ref_ptr<vsg::Object> ObjectFactory::create(const std::string& className)
{
    if (auto itr = _createMap.find(className); itr != _createMap.end())
    {
        debug("Using _createMap for ", className);
        return (itr->second)();
    }

    warn("ObjectFactory::create(", className, ") failed to find means to create object");
    return vsg::ref_ptr<vsg::Object>();
}
