/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/ConstVisitor.h>
#include <vsg/core/External.h>
#include <vsg/core/Objects.h>

#include <vsg/nodes/Commands.h>
#include <vsg/nodes/CullGroup.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexIndexDraw.h>

#include <vsg/vk/BindIndexBuffer.h>
#include <vsg/vk/BindVertexBuffers.h>
#include <vsg/vk/Command.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/ComputePipeline.h>
#include <vsg/vk/Descriptor.h>
#include <vsg/vk/DescriptorSet.h>
#include <vsg/vk/GraphicsPipeline.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/ResourceHints.h>

#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/TouchEvent.h>

#include <vsg/viewer/CommandGraph.h>
#include <vsg/viewer/RenderGraph.h>

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

////////////////////////////////////////////////////////////////////////////////
//
// Values
//
void ConstVisitor::apply(const stringValue& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const boolValue& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const intValue& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const uintValue& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const floatValue& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const doubleValue& value)
{
    apply(static_cast<const Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Arrays
//
void ConstVisitor::apply(const ubyteArray& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ushortArray& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const uintArray& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const floatArray& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const doubleArray& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec2Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec3Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec4Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec2Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec3Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec4Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ubvec2Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ubvec3Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ubvec4Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const mat4Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dmat4Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const block64Array& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const block128Array& value)
{
    apply(static_cast<const Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Array2Ds
//
void ConstVisitor::apply(const ubyteArray2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ushortArray2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const uintArray2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const floatArray2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const doubleArray2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec2Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec3Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec4Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec2Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec3Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec4Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ubvec2Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ubvec3Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ubvec4Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const block64Array2D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const block128Array2D& value)
{
    apply(static_cast<const Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Array3Ds
//
void ConstVisitor::apply(const ubyteArray3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ushortArray3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const uintArray3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const floatArray3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const doubleArray3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec2Array3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec3Array3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const vec4Array3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec2Array3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec3Array3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const dvec4Array3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ubvec2Array3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ubvec3Array3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ubvec4Array3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const block64Array3D& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const block128Array3D& value)
{
    apply(static_cast<const Object&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Nodes
//
void ConstVisitor::apply(const Node& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const Commands& value)
{
    apply(static_cast<const Object&>(value));
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
void ConstVisitor::apply(const MatrixTransform& value)
{
    apply(static_cast<const Group&>(value));
}
void ConstVisitor::apply(const Geometry& value)
{
    apply(static_cast<const Command&>(value));
}
void ConstVisitor::apply(const VertexIndexDraw& value)
{
    apply(static_cast<const Command&>(value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Vulkan Object
//
void ConstVisitor::apply(const Command& value)
{
    apply(static_cast<const Node&>(value));
}
void ConstVisitor::apply(const StateCommand& value)
{
    apply(static_cast<const Command&>(value));
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
void ConstVisitor::apply(const DescriptorSet& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const Descriptor& value)
{
    apply(static_cast<const Object&>(value));
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
void ConstVisitor::apply(const GraphicsPipeline& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const ComputePipeline& value)
{
    apply(static_cast<const Object&>(value));
}
void ConstVisitor::apply(const GraphicsPipelineState& value)
{
    apply(static_cast<const Object&>(value));
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
void ConstVisitor::apply(const ResourceHints& value)
{
    apply(static_cast<const Object&>(value));
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
// Viewer classes
//
void ConstVisitor::apply(const CommandGraph& cg)
{
    apply(static_cast<const Group&>(cg));
}
void ConstVisitor::apply(const RenderGraph& rg)
{
    apply(static_cast<const Group&>(rg));
}

////////////////////////////////////////////////////////////////////////////////
//
// General classes
//
void ConstVisitor::apply(const FrameStamp& fs)
{
    apply(static_cast<const Object&>(fs));
}
