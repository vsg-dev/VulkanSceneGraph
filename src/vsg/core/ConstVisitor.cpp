/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/all.h>

using namespace vsg;

ConstVisitor::ConstVisitor()
{
}

void ConstVisitor::apply(const Object&)
{
}

void ConstVisitor::apply(const Objects& value)
{
    apply(static_cast<const Object&>(value));
}

void ConstVisitor::apply(const External& value)
{
    apply(static_cast<const Object&>(value));
}

void ConstVisitor::apply(const Data& value)
{
    apply(static_cast<const Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Values
//
void ConstVisitor::apply(const stringValue& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const wstringValue& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const boolValue& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const intValue& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const uintValue& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const floatValue& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const doubleValue& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const vec2Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const vec3Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const vec4Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const dvec2Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const dvec3Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const dvec4Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const bvec2Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const bvec3Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const bvec4Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubvec2Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubvec3Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubvec4Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const svec2Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const svec3Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const svec4Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const usvec2Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const usvec3Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const usvec4Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ivec2Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ivec3Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ivec4Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const uivec2Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const uivec3Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const uivec4Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const mat4Value& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const dmat4Value& value)
{
    apply(static_cast<const Data&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Arrays
//
void ConstVisitor::apply(const byteArray& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubyteArray& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const shortArray& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ushortArray& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const intArray& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const uintArray& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const floatArray& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const doubleArray& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const vec2Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const vec3Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const vec4Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const dvec2Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const dvec3Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const dvec4Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const bvec2Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const bvec3Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const bvec4Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const svec2Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const svec3Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const svec4Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ivec2Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ivec3Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ivec4Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubvec2Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubvec3Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubvec4Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const usvec2Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const usvec3Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const usvec4Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const uivec2Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const uivec3Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const uivec4Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const mat4Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const dmat4Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const block64Array& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const block128Array& value)
{
    apply(static_cast<const Data&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Array2Ds
//
void ConstVisitor::apply(const byteArray2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubyteArray2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const shortArray2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ushortArray2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const intArray2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const uintArray2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const floatArray2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const doubleArray2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const vec2Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const vec3Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const vec4Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const dvec2Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const dvec3Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const dvec4Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const bvec2Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const bvec3Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const bvec4Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const svec2Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const svec3Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const svec4Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ivec2Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ivec3Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ivec4Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubvec2Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubvec3Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubvec4Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const usvec2Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const usvec3Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const usvec4Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const uivec2Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const uivec3Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const uivec4Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const block64Array2D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const block128Array2D& value)
{
    apply(static_cast<const Data&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Array3Ds
//
void ConstVisitor::apply(const byteArray3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubyteArray3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const shortArray3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ushortArray3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const intArray3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const uintArray3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const floatArray3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const doubleArray3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const vec2Array3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const vec3Array3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const vec4Array3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const dvec2Array3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const dvec3Array3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const dvec4Array3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubvec2Array3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubvec3Array3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const ubvec4Array3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const block64Array3D& value)
{
    apply(static_cast<const Data&>(value));
}
void ConstVisitor::apply(const block128Array3D& value)
{
    apply(static_cast<const Data&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Nodes
//
void ConstVisitor::apply(const Node& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const Compilable& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const Commands& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const Group& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const QuadGroup& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const LOD& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const PagedLOD& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const StateGroup& value)
{
    apply(static_cast<const Group&>(value));
}
void ConstVisitor::apply(const CullGroup& value)
{
    apply(static_cast<const Group&>(value));
}
void ConstVisitor::apply(const CullNode& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const Transform& value)
{
    apply(static_cast<const Group&>(value));
}
void ConstVisitor::apply(const MatrixTransform& value)
{
    apply(static_cast<const Transform&>(value));
}
void ConstVisitor::apply(const CoordinateFrame& value)
{
    apply(static_cast<const Transform&>(value));
}
void ConstVisitor::apply(const Geometry& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const VertexDraw& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const VertexIndexDraw& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const DepthSorted& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const Layer& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const Bin& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const Switch& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const Light& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const AmbientLight& value)
{
    apply(static_cast<const Light&>(value));
}
void ConstVisitor::apply(const DirectionalLight& value)
{
    apply(static_cast<const Light&>(value));
}
void ConstVisitor::apply(const PointLight& value)
{
    apply(static_cast<const Light&>(value));
}
void ConstVisitor::apply(const SpotLight& value)
{
    apply(static_cast<const Light&>(value));
}
void ConstVisitor::apply(const InstrumentationNode& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const RegionOfInterest& value)
{
    apply(static_cast<const Node&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Text Objects
//
void ConstVisitor::apply(const Text& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const TextGroup& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const TextTechnique& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const TextLayout& value)
{
    apply(static_cast<const Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Animation Objects/Nodes
//
void ConstVisitor::apply(const Animation& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const AnimationGroup& value)
{
    apply(static_cast<const Group&>(value));
}
void ConstVisitor::apply(const AnimationSampler& sampler)
{
    apply(static_cast<const Object&>(sampler));
}
void ConstVisitor::apply(const JointSampler& sampler)
{
    apply(static_cast<const AnimationSampler&>(sampler));
}
void ConstVisitor::apply(const MorphSampler& sampler)
{
    apply(static_cast<const AnimationSampler&>(sampler));
}
void ConstVisitor::apply(const TransformSampler& sampler)
{
    apply(static_cast<const AnimationSampler&>(sampler));
}
void ConstVisitor::apply(const CameraSampler& sampler)
{
    apply(static_cast<const AnimationSampler&>(sampler));
}
void ConstVisitor::apply(const Joint& value)
{
    apply(static_cast<const Node&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Vulkan Objects
//
void ConstVisitor::apply(const BufferInfo& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ImageInfo& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ImageView& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const Image& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const Command& value)
{
    apply(static_cast<const Compilable&>(value));
}
void ConstVisitor::apply(const StateCommand& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const StateSwitch& value)
{
    apply(static_cast<const StateCommand&>(value));
}
void ConstVisitor::apply(const CommandBuffer& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const RenderPass& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const BindDescriptorSet& value)
{
    apply(static_cast<const StateCommand&>(value));
}
void ConstVisitor::apply(const BindDescriptorSets& value)
{
    apply(static_cast<const StateCommand&>(value));
}
void ConstVisitor::apply(const BindViewDescriptorSets& value)
{
    apply(static_cast<const StateCommand&>(value));
}
void ConstVisitor::apply(const DescriptorSet& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const Descriptor& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const DescriptorBuffer& value)
{
    apply(static_cast<const Descriptor&>(value));
}
void ConstVisitor::apply(const DescriptorImage& value)
{
    apply(static_cast<const Descriptor&>(value));
}
void ConstVisitor::apply(const BindVertexBuffers& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const BindIndexBuffer& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const BindComputePipeline& value)
{
    apply(static_cast<const StateCommand&>(value));
}
void ConstVisitor::apply(const BindGraphicsPipeline& value)
{
    apply(static_cast<const StateCommand&>(value));
}
void ConstVisitor::apply(const BindRayTracingPipeline& value)
{
    apply(static_cast<const StateCommand&>(value));
}
void ConstVisitor::apply(const GraphicsPipeline& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ComputePipeline& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const RayTracingPipeline& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const GraphicsPipelineState& value)
{
    apply(static_cast<const StateCommand&>(value));
}
void ConstVisitor::apply(const ShaderStage& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const VertexInputState& value)
{
    apply(static_cast<const GraphicsPipelineState&>(value));
}
void ConstVisitor::apply(const InputAssemblyState& value)
{
    apply(static_cast<const GraphicsPipelineState&>(value));
}
void ConstVisitor::apply(const TessellationState& value)
{
    apply(static_cast<const GraphicsPipelineState&>(value));
}
void ConstVisitor::apply(const ViewportState& value)
{
    apply(static_cast<const GraphicsPipelineState&>(value));
}
void ConstVisitor::apply(const RasterizationState& value)
{
    apply(static_cast<const GraphicsPipelineState&>(value));
}
void ConstVisitor::apply(const MultisampleState& value)
{
    apply(static_cast<const GraphicsPipelineState&>(value));
}
void ConstVisitor::apply(const DepthStencilState& value)
{
    apply(static_cast<const GraphicsPipelineState&>(value));
}
void ConstVisitor::apply(const ColorBlendState& value)
{
    apply(static_cast<const GraphicsPipelineState&>(value));
}
void ConstVisitor::apply(const DynamicState& value)
{
    apply(static_cast<const GraphicsPipelineState&>(value));
}
void ConstVisitor::apply(const ResourceHints& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const Draw& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const DrawIndexed& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const ClearAttachments& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const ClearColorImage& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const ClearDepthStencilImage& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const QueryPool& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ResetQueryPool& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const BeginQuery& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const EndQuery& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const WriteTimestamp& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const CopyQueryPoolResults& value)
{
    apply(static_cast<const Command&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Mesh shading
//
void ConstVisitor::apply(const DrawMeshTasks& dmt)
{
    apply(static_cast<const Command&>(dmt));
}
void ConstVisitor::apply(const DrawMeshTasksIndirect& dmti)
{
    apply(static_cast<const Command&>(dmti));
}
void ConstVisitor::apply(const DrawMeshTasksIndirectCount& dmtic)
{
    apply(static_cast<const Command&>(dmtic));
}

////////////////////////////////////////////////////////////////////////////////
//
// UI Events
//
void ConstVisitor::apply(const UIEvent& event)
{
    apply(static_cast<const Object&>(event));
}
void ConstVisitor::apply(const WindowEvent& event)
{
    apply(static_cast<const UIEvent&>(event));
}
void ConstVisitor::apply(const ExposeWindowEvent& event)
{
    apply(static_cast<const WindowEvent&>(event));
}
void ConstVisitor::apply(const ConfigureWindowEvent& event)
{
    apply(static_cast<const WindowEvent&>(event));
}
void ConstVisitor::apply(const CloseWindowEvent& event)
{
    apply(static_cast<const WindowEvent&>(event));
}
void ConstVisitor::apply(const FocusInEvent& event)
{
    apply(static_cast<const WindowEvent&>(event));
}
void ConstVisitor::apply(const FocusOutEvent& event)
{
    apply(static_cast<const WindowEvent&>(event));
}
void ConstVisitor::apply(const KeyEvent& event)
{
    apply(static_cast<const WindowEvent&>(event));
}
void ConstVisitor::apply(const KeyPressEvent& event)
{
    apply(static_cast<const KeyEvent&>(event));
}
void ConstVisitor::apply(const KeyReleaseEvent& event)
{
    apply(static_cast<const KeyEvent&>(event));
}
void ConstVisitor::apply(const PointerEvent& event)
{
    apply(static_cast<const WindowEvent&>(event));
}
void ConstVisitor::apply(const ButtonPressEvent& event)
{
    apply(static_cast<const PointerEvent&>(event));
}
void ConstVisitor::apply(const ButtonReleaseEvent& event)
{
    apply(static_cast<const PointerEvent&>(event));
}
void ConstVisitor::apply(const MoveEvent& event)
{
    apply(static_cast<const PointerEvent&>(event));
}
void ConstVisitor::apply(const TouchEvent& event)
{
    apply(static_cast<const WindowEvent&>(event));
}
void ConstVisitor::apply(const TouchDownEvent& event)
{
    apply(static_cast<const TouchEvent&>(event));
}
void ConstVisitor::apply(const TouchUpEvent& event)
{
    apply(static_cast<const TouchEvent&>(event));
}
void ConstVisitor::apply(const TouchMoveEvent& event)
{
    apply(static_cast<const TouchEvent&>(event));
}
void ConstVisitor::apply(const ScrollWheelEvent& event)
{
    apply(static_cast<const WindowEvent&>(event));
}
void ConstVisitor::apply(const TerminateEvent& event)
{
    apply(static_cast<const UIEvent&>(event));
}
void ConstVisitor::apply(const FrameEvent& event)
{
    apply(static_cast<const UIEvent&>(event));
}

////////////////////////////////////////////////////////////////////////////////
//
// util classes
//
void ConstVisitor::apply(const ShaderCompileSettings& shaderCompileSettings)
{
    apply(static_cast<const Object&>(shaderCompileSettings));
}

////////////////////////////////////////////////////////////////////////////////
//
// Viewer classes
//
void ConstVisitor::apply(const Camera& camera)
{
    apply(static_cast<const Object&>(camera));
}
void ConstVisitor::apply(const CommandGraph& cg)
{
    apply(static_cast<const Group&>(cg));
}
void ConstVisitor::apply(const SecondaryCommandGraph& cg)
{
    apply(static_cast<const CommandGraph&>(cg));
}
void ConstVisitor::apply(const RenderGraph& rg)
{
    apply(static_cast<const Group&>(rg));
}
void ConstVisitor::apply(const View& view)
{
    apply(static_cast<const Group&>(view));
}
void ConstVisitor::apply(const Viewer& viewer)
{
    apply(static_cast<const Object&>(viewer));
}
void ConstVisitor::apply(const ViewMatrix& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const LookAt& value)
{
    apply(static_cast<const ViewMatrix&>(value));
}
void ConstVisitor::apply(const LookDirection& value)
{
    apply(static_cast<const ViewMatrix&>(value));
}
void ConstVisitor::apply(const RelativeViewMatrix& value)
{
    apply(static_cast<const ViewMatrix&>(value));
}
void ConstVisitor::apply(const TrackingViewMatrix& value)
{
    apply(static_cast<const ViewMatrix&>(value));
}
void ConstVisitor::apply(const ProjectionMatrix& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const Perspective& value)
{
    apply(static_cast<const ProjectionMatrix&>(value));
}
void ConstVisitor::apply(const Orthographic& value)
{
    apply(static_cast<const ProjectionMatrix&>(value));
}
void ConstVisitor::apply(const RelativeProjection& value)
{
    apply(static_cast<const ProjectionMatrix&>(value));
}
void ConstVisitor::apply(const EllipsoidPerspective& value)
{
    apply(static_cast<const ProjectionMatrix&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// General classes
//
void ConstVisitor::apply(const FrameStamp& fs)
{
    apply(static_cast<const Object&>(fs));
}
