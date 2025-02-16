/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/all.h>

using namespace vsg;

Visitor::Visitor()
{
}

void Visitor::apply(Object&)
{
}

void Visitor::apply(Objects& value)
{
    apply(static_cast<Object&>(value));
}

void Visitor::apply(External& value)
{
    apply(static_cast<Object&>(value));
}

void Visitor::apply(Data& value)
{
    apply(static_cast<Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Values
//
void Visitor::apply(stringValue& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(wstringValue& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(boolValue& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(intValue& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(uintValue& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(floatValue& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(doubleValue& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(vec2Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(vec3Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(vec4Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(dvec2Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(dvec3Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(dvec4Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(bvec2Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(bvec3Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(bvec4Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubvec2Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubvec3Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubvec4Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(svec2Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(svec3Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(svec4Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(usvec2Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(usvec3Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(usvec4Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ivec2Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ivec3Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ivec4Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(uivec2Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(uivec3Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(uivec4Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(mat4Value& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(dmat4Value& value)
{
    apply(static_cast<Data&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Arrays
//
void Visitor::apply(byteArray& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubyteArray& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(shortArray& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ushortArray& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(intArray& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(uintArray& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(floatArray& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(doubleArray& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(vec2Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(vec3Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(vec4Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(dvec2Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(dvec3Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(dvec4Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(bvec2Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(bvec3Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(bvec4Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(svec2Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(svec3Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(svec4Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ivec2Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ivec3Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ivec4Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubvec2Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubvec3Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubvec4Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(usvec2Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(usvec3Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(usvec4Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(uivec2Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(uivec3Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(uivec4Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(mat4Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(dmat4Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(block64Array& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(block128Array& value)
{
    apply(static_cast<Data&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Array2Ds
//
void Visitor::apply(byteArray2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubyteArray2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(shortArray2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ushortArray2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(intArray2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(uintArray2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(floatArray2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(doubleArray2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(vec2Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(vec3Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(vec4Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(dvec2Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(dvec3Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(dvec4Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(bvec2Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(bvec3Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(bvec4Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(svec2Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(svec3Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(svec4Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ivec2Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ivec3Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ivec4Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubvec2Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubvec3Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubvec4Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(usvec2Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(usvec3Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(usvec4Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(uivec2Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(uivec3Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(uivec4Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(block64Array2D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(block128Array2D& value)
{
    apply(static_cast<Data&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Array3Ds
//
void Visitor::apply(byteArray3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubyteArray3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(shortArray3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ushortArray3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(intArray3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(uintArray3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(floatArray3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(doubleArray3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(vec2Array3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(vec3Array3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(vec4Array3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(dvec2Array3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(dvec3Array3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(dvec4Array3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubvec2Array3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubvec3Array3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(ubvec4Array3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(block64Array3D& value)
{
    apply(static_cast<Data&>(value));
}
void Visitor::apply(block128Array3D& value)
{
    apply(static_cast<Data&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Nodes
//
void Visitor::apply(Node& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(Compilable& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(Commands& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(Group& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(QuadGroup& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(LOD& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(PagedLOD& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(StateGroup& value)
{
    apply(static_cast<Group&>(value));
}
void Visitor::apply(CullGroup& value)
{
    apply(static_cast<Group&>(value));
}
void Visitor::apply(CullNode& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(Transform& value)
{
    apply(static_cast<Group&>(value));
}
void Visitor::apply(MatrixTransform& value)
{
    apply(static_cast<Transform&>(value));
}
void Visitor::apply(CoordinateFrame& value)
{
    apply(static_cast<Transform&>(value));
}
void Visitor::apply(Geometry& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(VertexDraw& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(VertexIndexDraw& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(DepthSorted& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(Layer& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(Bin& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(Switch& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(Light& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(AmbientLight& value)
{
    apply(static_cast<Light&>(value));
}
void Visitor::apply(DirectionalLight& value)
{
    apply(static_cast<Light&>(value));
}
void Visitor::apply(PointLight& value)
{
    apply(static_cast<Light&>(value));
}
void Visitor::apply(SpotLight& value)
{
    apply(static_cast<Light&>(value));
}
void Visitor::apply(InstrumentationNode& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(RegionOfInterest& value)
{
    apply(static_cast<Node&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Text Objects
//
void Visitor::apply(Text& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(TextGroup& value)
{
    apply(static_cast<Node&>(value));
}
void Visitor::apply(TextTechnique& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(TextLayout& value)
{
    apply(static_cast<Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Animation Objects/Nodes
//
void Visitor::apply(Animation& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(AnimationGroup& value)
{
    apply(static_cast<Group&>(value));
}
void Visitor::apply(AnimationSampler& sampler)
{
    apply(static_cast<Object&>(sampler));
}
void Visitor::apply(JointSampler& sampler)
{
    apply(static_cast<AnimationSampler&>(sampler));
}
void Visitor::apply(MorphSampler& sampler)
{
    apply(static_cast<AnimationSampler&>(sampler));
}
void Visitor::apply(TransformSampler& sampler)
{
    apply(static_cast<AnimationSampler&>(sampler));
}
void Visitor::apply(CameraSampler& sampler)
{
    apply(static_cast<AnimationSampler&>(sampler));
}
void Visitor::apply(Joint& value)
{
    apply(static_cast<Node&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Vulkan Objects
//
void Visitor::apply(BufferInfo& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ImageInfo& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ImageView& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(Image& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(Command& value)
{
    apply(static_cast<Compilable&>(value));
}
void Visitor::apply(StateCommand& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(StateSwitch& value)
{
    apply(static_cast<StateCommand&>(value));
}
void Visitor::apply(CommandBuffer& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(RenderPass& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(BindDescriptorSet& value)
{
    apply(static_cast<StateCommand&>(value));
}
void Visitor::apply(BindDescriptorSets& value)
{
    apply(static_cast<StateCommand&>(value));
}
void Visitor::apply(BindViewDescriptorSets& value)
{
    apply(static_cast<StateCommand&>(value));
}
void Visitor::apply(Descriptor& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(DescriptorBuffer& value)
{
    apply(static_cast<Descriptor&>(value));
}
void Visitor::apply(DescriptorImage& value)
{
    apply(static_cast<Descriptor&>(value));
}
void Visitor::apply(DescriptorSet& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(BindVertexBuffers& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(BindIndexBuffer& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(BindComputePipeline& value)
{
    apply(static_cast<StateCommand&>(value));
}
void Visitor::apply(BindGraphicsPipeline& value)
{
    apply(static_cast<StateCommand&>(value));
}
void Visitor::apply(BindRayTracingPipeline& value)
{
    apply(static_cast<StateCommand&>(value));
}
void Visitor::apply(GraphicsPipeline& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ComputePipeline& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(RayTracingPipeline& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(GraphicsPipelineState& value)
{
    apply(static_cast<StateCommand&>(value));
}
void Visitor::apply(ShaderStage& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(VertexInputState& value)
{
    apply(static_cast<GraphicsPipelineState&>(value));
}
void Visitor::apply(InputAssemblyState& value)
{
    apply(static_cast<GraphicsPipelineState&>(value));
}
void Visitor::apply(TessellationState& value)
{
    apply(static_cast<GraphicsPipelineState&>(value));
}
void Visitor::apply(ViewportState& value)
{
    apply(static_cast<GraphicsPipelineState&>(value));
}
void Visitor::apply(RasterizationState& value)
{
    apply(static_cast<GraphicsPipelineState&>(value));
}
void Visitor::apply(MultisampleState& value)
{
    apply(static_cast<GraphicsPipelineState&>(value));
}
void Visitor::apply(DepthStencilState& value)
{
    apply(static_cast<GraphicsPipelineState&>(value));
}
void Visitor::apply(ColorBlendState& value)
{
    apply(static_cast<GraphicsPipelineState&>(value));
}
void Visitor::apply(DynamicState& value)
{
    apply(static_cast<GraphicsPipelineState&>(value));
}
void Visitor::apply(ResourceHints& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(Draw& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(DrawIndexed& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(ClearAttachments& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(ClearColorImage& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(ClearDepthStencilImage& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(QueryPool& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(ResetQueryPool& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(BeginQuery& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(EndQuery& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(WriteTimestamp& value)
{
    apply(static_cast<Command&>(value));
}
void Visitor::apply(CopyQueryPoolResults& value)
{
    apply(static_cast<Command&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Mesh shading
//
void Visitor::apply(DrawMeshTasks& dmt)
{
    apply(static_cast<Command&>(dmt));
}
void Visitor::apply(DrawMeshTasksIndirect& dmti)
{
    apply(static_cast<Command&>(dmti));
}
void Visitor::apply(DrawMeshTasksIndirectCount& dmtic)
{
    apply(static_cast<Command&>(dmtic));
}

////////////////////////////////////////////////////////////////////////////////
//
// UI Events
//
void Visitor::apply(UIEvent& event)
{
    apply(static_cast<Object&>(event));
}
void Visitor::apply(WindowEvent& event)
{
    apply(static_cast<UIEvent&>(event));
}
void Visitor::apply(ExposeWindowEvent& event)
{
    apply(static_cast<WindowEvent&>(event));
}
void Visitor::apply(ConfigureWindowEvent& event)
{
    apply(static_cast<WindowEvent&>(event));
}
void Visitor::apply(CloseWindowEvent& event)
{
    apply(static_cast<WindowEvent&>(event));
}
void Visitor::apply(FocusInEvent& event)
{
    apply(static_cast<WindowEvent&>(event));
}
void Visitor::apply(FocusOutEvent& event)
{
    apply(static_cast<WindowEvent&>(event));
}
void Visitor::apply(KeyEvent& event)
{
    apply(static_cast<WindowEvent&>(event));
}
void Visitor::apply(KeyPressEvent& event)
{
    apply(static_cast<KeyEvent&>(event));
}
void Visitor::apply(KeyReleaseEvent& event)
{
    apply(static_cast<KeyEvent&>(event));
}
void Visitor::apply(PointerEvent& event)
{
    apply(static_cast<WindowEvent&>(event));
}
void Visitor::apply(ButtonPressEvent& event)
{
    apply(static_cast<PointerEvent&>(event));
}
void Visitor::apply(ButtonReleaseEvent& event)
{
    apply(static_cast<PointerEvent&>(event));
}
void Visitor::apply(MoveEvent& event)
{
    apply(static_cast<PointerEvent&>(event));
}
void Visitor::apply(TouchEvent& event)
{
    apply(static_cast<WindowEvent&>(event));
}
void Visitor::apply(TouchDownEvent& event)
{
    apply(static_cast<TouchEvent&>(event));
}
void Visitor::apply(TouchUpEvent& event)
{
    apply(static_cast<TouchEvent&>(event));
}
void Visitor::apply(TouchMoveEvent& event)
{
    apply(static_cast<TouchEvent&>(event));
}
void Visitor::apply(ScrollWheelEvent& event)
{
    apply(static_cast<WindowEvent&>(event));
}
void Visitor::apply(TerminateEvent& event)
{
    apply(static_cast<UIEvent&>(event));
}
void Visitor::apply(FrameEvent& event)
{
    apply(static_cast<UIEvent&>(event));
}

////////////////////////////////////////////////////////////////////////////////
//
// util classes
//
void Visitor::apply(ShaderCompileSettings& shaderCompileSettings)
{
    apply(static_cast<Object&>(shaderCompileSettings));
}

////////////////////////////////////////////////////////////////////////////////
//
// Viewer classes
//
void Visitor::apply(Camera& camera)
{
    apply(static_cast<Object&>(camera));
}
void Visitor::apply(CommandGraph& cg)
{
    apply(static_cast<Group&>(cg));
}
void Visitor::apply(SecondaryCommandGraph& cg)
{
    apply(static_cast<CommandGraph&>(cg));
}
void Visitor::apply(RenderGraph& rg)
{
    apply(static_cast<Group&>(rg));
}
void Visitor::apply(View& view)
{
    apply(static_cast<Group&>(view));
}
void Visitor::apply(Viewer& viewer)
{
    apply(static_cast<Object&>(viewer));
}
void Visitor::apply(ViewMatrix& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(LookAt& value)
{
    apply(static_cast<ViewMatrix&>(value));
}
void Visitor::apply(LookDirection& value)
{
    apply(static_cast<ViewMatrix&>(value));
}
void Visitor::apply(RelativeViewMatrix& value)
{
    apply(static_cast<ViewMatrix&>(value));
}
void Visitor::apply(TrackingViewMatrix& value)
{
    apply(static_cast<ViewMatrix&>(value));
}
void Visitor::apply(ProjectionMatrix& value)
{
    apply(static_cast<Object&>(value));
}
void Visitor::apply(Perspective& value)
{
    apply(static_cast<ProjectionMatrix&>(value));
}
void Visitor::apply(Orthographic& value)
{
    apply(static_cast<ProjectionMatrix&>(value));
}
void Visitor::apply(RelativeProjection& value)
{
    apply(static_cast<ProjectionMatrix&>(value));
}
void Visitor::apply(EllipsoidPerspective& value)
{
    apply(static_cast<ProjectionMatrix&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// General classes
//
void Visitor::apply(FrameStamp& fs)
{
    apply(static_cast<Object&>(fs));
}
