#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Array.h>
#include <vsg/core/Array2D.h>
#include <vsg/core/Array3D.h>
#include <vsg/core/Mask.h>
#include <vsg/core/Value.h>

namespace vsg
{
    // forward declare core Objects
    class Objects;
    class External;

    // forward declare node classes
    class Node;
    class Commands;
    class Group;
    class QuadGroup;
    class LOD;
    class PagedLOD;
    class StateGroup;
    class CullGroup;
    class CullNode;
    class Transform;
    class MatrixTransform;
    class CoordinateFrame;
    class Geometry;
    class VertexDraw;
    class VertexIndexDraw;
    class DepthSorted;
    class Layer;
    class Bin;
    class Switch;
    class Light;
    class AmbientLight;
    class DirectionalLight;
    class PointLight;
    class SpotLight;
    class InstrumentationNode;
    class RegionOfInterest;

    // forward declare text classes
    class Text;
    class TextGroup;
    class TextTechnique;
    class TextLayout;

    // forward declare animation classes
    class Animation;
    class AnimationGroup;
    class AnimationSampler;
    class TransformSampler;
    class CameraSampler;
    class MorphSampler;
    class JointSampler;
    class Joint;

    // forward declare vulkan classes
    class BufferInfo;
    class ImageInfo;
    class ImageView;
    class Image;
    class Compilable;
    class Command;
    class StateCommand;
    class StateSwitch;
    class CommandBuffer;
    class RenderPass;
    class BindDescriptorSet;
    class BindDescriptorSets;
    class BindViewDescriptorSets;
    class Descriptor;
    class DescriptorBuffer;
    class DescriptorImage;
    class DescriptorSet;
    class BindVertexBuffers;
    class BindIndexBuffer;
    class BindComputePipeline;
    class BindGraphicsPipeline;
    class BindRayTracingPipeline;
    class GraphicsPipeline;
    class ComputePipeline;
    class RayTracingPipeline;
    class Draw;
    class DrawIndexed;
    class ShaderStage;
    class GraphicsPipelineState;
    class VertexInputState;
    class InputAssemblyState;
    class TessellationState;
    class ViewportState;
    class RasterizationState;
    class MultisampleState;
    class DepthStencilState;
    class ColorBlendState;
    class DynamicState;
    class ResourceHints;
    class ClearAttachments;
    class ClearColorImage;
    class ClearDepthStencilImage;
    class QueryPool;
    class ResetQueryPool;
    class BeginQuery;
    class EndQuery;
    class WriteTimestamp;
    class CopyQueryPoolResults;

    // forward declare rtx classes
    class DrawMeshTasks;
    class DrawMeshTasksIndirect;
    class DrawMeshTasksIndirectCount;

    // forward declare ui events classes
    class UIEvent;
    class WindowEvent;
    class ExposeWindowEvent;
    class ConfigureWindowEvent;
    class CloseWindowEvent;
    class FocusInEvent;
    class FocusOutEvent;
    class KeyEvent;
    class KeyPressEvent;
    class KeyReleaseEvent;
    class PointerEvent;
    class ButtonPressEvent;
    class ButtonReleaseEvent;
    class MoveEvent;
    class TouchEvent;
    class TouchDownEvent;
    class TouchUpEvent;
    class TouchMoveEvent;
    class ScrollWheelEvent;
    class TerminateEvent;
    class FrameEvent;

    // forward declare util classes
    class ShaderCompileSettings;

    // forward declare viewer classes
    class Camera;
    class CommandGraph;
    class SecondaryCommandGraph;
    class RenderGraph;
    class View;
    class Viewer;
    class ViewMatrix;
    class LookAt;
    class LookDirection;
    class RelativeViewMatrix;
    class TrackingViewMatrix;
    class ProjectionMatrix;
    class Perspective;
    class Orthographic;
    class RelativeProjection;
    class EllipsoidPerspective;

    // forward declare general classes
    class FrameStamp;
    class Instrumentation;

    class VSG_DECLSPEC Visitor : public Object
    {
    public:
        Visitor();

        Visitor(const Visitor& rhs, const CopyOp& copyop = {}) :
            Object(rhs, copyop),
            traversalMask(rhs.traversalMask),
            overrideMask(rhs.overrideMask) {}

        Mask traversalMask = MASK_ALL;
        Mask overrideMask = MASK_OFF;

        virtual Instrumentation* getInstrumentation() { return nullptr; }

        virtual void apply(Object&);
        virtual void apply(Objects&);
        virtual void apply(External&);
        virtual void apply(Data&);

        // Values
        virtual void apply(stringValue&);
        virtual void apply(wstringValue&);
        virtual void apply(boolValue&);
        virtual void apply(intValue&);
        virtual void apply(uintValue&);
        virtual void apply(floatValue&);
        virtual void apply(doubleValue&);
        virtual void apply(vec2Value&);
        virtual void apply(vec3Value&);
        virtual void apply(vec4Value&);
        virtual void apply(dvec2Value&);
        virtual void apply(dvec3Value&);
        virtual void apply(dvec4Value&);
        virtual void apply(bvec2Value&);
        virtual void apply(bvec3Value&);
        virtual void apply(bvec4Value&);
        virtual void apply(ubvec2Value&);
        virtual void apply(ubvec3Value&);
        virtual void apply(ubvec4Value&);
        virtual void apply(svec2Value&);
        virtual void apply(svec3Value&);
        virtual void apply(svec4Value&);
        virtual void apply(usvec2Value&);
        virtual void apply(usvec3Value&);
        virtual void apply(usvec4Value&);
        virtual void apply(ivec2Value&);
        virtual void apply(ivec3Value&);
        virtual void apply(ivec4Value&);
        virtual void apply(uivec2Value&);
        virtual void apply(uivec3Value&);
        virtual void apply(uivec4Value&);
        virtual void apply(mat4Value&);
        virtual void apply(dmat4Value&);

        // Arrays
        virtual void apply(byteArray&);
        virtual void apply(ubyteArray&);
        virtual void apply(shortArray&);
        virtual void apply(ushortArray&);
        virtual void apply(intArray&);
        virtual void apply(uintArray&);
        virtual void apply(floatArray&);
        virtual void apply(doubleArray&);
        virtual void apply(vec2Array&);
        virtual void apply(vec3Array&);
        virtual void apply(vec4Array&);
        virtual void apply(dvec2Array&);
        virtual void apply(dvec3Array&);
        virtual void apply(dvec4Array&);
        virtual void apply(bvec2Array&);
        virtual void apply(bvec3Array&);
        virtual void apply(bvec4Array&);
        virtual void apply(svec2Array&);
        virtual void apply(svec3Array&);
        virtual void apply(svec4Array&);
        virtual void apply(ivec2Array&);
        virtual void apply(ivec3Array&);
        virtual void apply(ivec4Array&);
        virtual void apply(ubvec2Array&);
        virtual void apply(ubvec3Array&);
        virtual void apply(ubvec4Array&);
        virtual void apply(usvec2Array&);
        virtual void apply(usvec3Array&);
        virtual void apply(usvec4Array&);
        virtual void apply(uivec2Array&);
        virtual void apply(uivec3Array&);
        virtual void apply(uivec4Array&);
        virtual void apply(mat4Array&);
        virtual void apply(dmat4Array&);
        virtual void apply(block64Array&);
        virtual void apply(block128Array&);

        // Array2Ds
        virtual void apply(byteArray2D&);
        virtual void apply(ubyteArray2D&);
        virtual void apply(shortArray2D&);
        virtual void apply(ushortArray2D&);
        virtual void apply(intArray2D&);
        virtual void apply(uintArray2D&);
        virtual void apply(floatArray2D&);
        virtual void apply(doubleArray2D&);
        virtual void apply(vec2Array2D&);
        virtual void apply(vec3Array2D&);
        virtual void apply(vec4Array2D&);
        virtual void apply(dvec2Array2D&);
        virtual void apply(dvec3Array2D&);
        virtual void apply(dvec4Array2D&);
        virtual void apply(bvec2Array2D&);
        virtual void apply(bvec3Array2D&);
        virtual void apply(bvec4Array2D&);
        virtual void apply(svec2Array2D&);
        virtual void apply(svec3Array2D&);
        virtual void apply(svec4Array2D&);
        virtual void apply(ivec2Array2D&);
        virtual void apply(ivec3Array2D&);
        virtual void apply(ivec4Array2D&);
        virtual void apply(ubvec2Array2D&);
        virtual void apply(ubvec3Array2D&);
        virtual void apply(ubvec4Array2D&);
        virtual void apply(usvec2Array2D&);
        virtual void apply(usvec3Array2D&);
        virtual void apply(usvec4Array2D&);
        virtual void apply(uivec2Array2D&);
        virtual void apply(uivec3Array2D&);
        virtual void apply(uivec4Array2D&);
        virtual void apply(block64Array2D&);
        virtual void apply(block128Array2D&);

        // Array3Ds
        virtual void apply(byteArray3D&);
        virtual void apply(ubyteArray3D&);
        virtual void apply(shortArray3D&);
        virtual void apply(ushortArray3D&);
        virtual void apply(intArray3D&);
        virtual void apply(uintArray3D&);
        virtual void apply(floatArray3D&);
        virtual void apply(doubleArray3D&);
        virtual void apply(vec2Array3D&);
        virtual void apply(vec3Array3D&);
        virtual void apply(vec4Array3D&);
        virtual void apply(dvec2Array3D&);
        virtual void apply(dvec3Array3D&);
        virtual void apply(dvec4Array3D&);
        virtual void apply(ubvec2Array3D&);
        virtual void apply(ubvec3Array3D&);
        virtual void apply(ubvec4Array3D&);
        virtual void apply(block64Array3D&);
        virtual void apply(block128Array3D&);

        // Nodes
        virtual void apply(Node&);
        virtual void apply(Compilable&);
        virtual void apply(Commands&);
        virtual void apply(Group&);
        virtual void apply(QuadGroup&);
        virtual void apply(LOD&);
        virtual void apply(PagedLOD&);
        virtual void apply(StateGroup&);
        virtual void apply(CullGroup&);
        virtual void apply(CullNode&);
        virtual void apply(Transform&);
        virtual void apply(MatrixTransform&);
        virtual void apply(CoordinateFrame&);
        virtual void apply(Geometry&);
        virtual void apply(VertexDraw&);
        virtual void apply(VertexIndexDraw&);
        virtual void apply(DepthSorted&);
        virtual void apply(Layer&);
        virtual void apply(Bin&);
        virtual void apply(Switch&);
        virtual void apply(Light&);
        virtual void apply(AmbientLight&);
        virtual void apply(DirectionalLight&);
        virtual void apply(PointLight&);
        virtual void apply(SpotLight&);
        virtual void apply(InstrumentationNode&);
        virtual void apply(RegionOfInterest&);

        // text
        virtual void apply(Text&);
        virtual void apply(TextGroup&);
        virtual void apply(TextTechnique&);
        virtual void apply(TextLayout&);

        // animation
        virtual void apply(Animation&);
        virtual void apply(AnimationGroup&);
        virtual void apply(AnimationSampler&);
        virtual void apply(JointSampler&);
        virtual void apply(MorphSampler&);
        virtual void apply(TransformSampler&);
        virtual void apply(CameraSampler&);
        virtual void apply(Joint&);

        // Vulkan nodes
        virtual void apply(BufferInfo&);
        virtual void apply(ImageInfo&);
        virtual void apply(ImageView&);
        virtual void apply(Image&);
        virtual void apply(Command&);
        virtual void apply(StateCommand&);
        virtual void apply(StateSwitch&);
        virtual void apply(CommandBuffer&);
        virtual void apply(RenderPass&);
        virtual void apply(BindDescriptorSet&);
        virtual void apply(BindDescriptorSets&);
        virtual void apply(BindViewDescriptorSets&);
        virtual void apply(Descriptor&);
        virtual void apply(DescriptorBuffer&);
        virtual void apply(DescriptorImage&);
        virtual void apply(DescriptorSet&);
        virtual void apply(BindVertexBuffers&);
        virtual void apply(BindIndexBuffer&);
        virtual void apply(BindComputePipeline&);
        virtual void apply(BindGraphicsPipeline&);
        virtual void apply(BindRayTracingPipeline&);
        virtual void apply(GraphicsPipeline&);
        virtual void apply(ComputePipeline&);
        virtual void apply(RayTracingPipeline&);
        virtual void apply(GraphicsPipelineState&);
        virtual void apply(ShaderStage&);
        virtual void apply(VertexInputState&);
        virtual void apply(InputAssemblyState&);
        virtual void apply(TessellationState&);
        virtual void apply(ViewportState&);
        virtual void apply(RasterizationState&);
        virtual void apply(MultisampleState&);
        virtual void apply(DepthStencilState&);
        virtual void apply(ColorBlendState&);
        virtual void apply(DynamicState&);
        virtual void apply(ResourceHints&);
        virtual void apply(Draw&);
        virtual void apply(DrawIndexed&);
        virtual void apply(ClearAttachments&);
        virtual void apply(ClearColorImage&);
        virtual void apply(ClearDepthStencilImage&);
        virtual void apply(QueryPool&);
        virtual void apply(ResetQueryPool&);
        virtual void apply(BeginQuery&);
        virtual void apply(EndQuery&);
        virtual void apply(WriteTimestamp&);
        virtual void apply(CopyQueryPoolResults&);

        // mesh shading classes
        virtual void apply(DrawMeshTasks&);
        virtual void apply(DrawMeshTasksIndirect&);
        virtual void apply(DrawMeshTasksIndirectCount&);

        // ui events
        virtual void apply(UIEvent&);
        virtual void apply(WindowEvent&);
        virtual void apply(ExposeWindowEvent&);
        virtual void apply(ConfigureWindowEvent&);
        virtual void apply(CloseWindowEvent&);
        virtual void apply(FocusInEvent&);
        virtual void apply(FocusOutEvent&);
        virtual void apply(KeyEvent&);
        virtual void apply(KeyPressEvent&);
        virtual void apply(KeyReleaseEvent&);
        virtual void apply(PointerEvent&);
        virtual void apply(ButtonPressEvent&);
        virtual void apply(ButtonReleaseEvent&);
        virtual void apply(MoveEvent&);
        virtual void apply(TouchEvent&);
        virtual void apply(TouchDownEvent&);
        virtual void apply(TouchUpEvent&);
        virtual void apply(TouchMoveEvent&);
        virtual void apply(ScrollWheelEvent&);
        virtual void apply(TerminateEvent&);
        virtual void apply(FrameEvent&);

        // utils
        virtual void apply(ShaderCompileSettings&);

        // app
        virtual void apply(Camera&);
        virtual void apply(CommandGraph&);
        virtual void apply(SecondaryCommandGraph&);
        virtual void apply(RenderGraph&);
        virtual void apply(View&);
        virtual void apply(Viewer&);
        virtual void apply(ViewMatrix&);
        virtual void apply(LookAt&);
        virtual void apply(LookDirection&);
        virtual void apply(RelativeViewMatrix&);
        virtual void apply(TrackingViewMatrix&);
        virtual void apply(ProjectionMatrix&);
        virtual void apply(Perspective&);
        virtual void apply(Orthographic&);
        virtual void apply(RelativeProjection&);
        virtual void apply(EllipsoidPerspective&);

        // general classes
        virtual void apply(FrameStamp&);

        bool is_compatible(const std::type_info& type) const noexcept override { return typeid(Visitor) == type || Object::is_compatible(type); }
    };

    // provide Value<>::accept() implementation
    template<typename T>
    void Value<T>::accept(Visitor& visitor) { visitor.apply(*this); }

    // provide Array<>::accept() implementation
    template<typename T>
    void Array<T>::accept(Visitor& visitor) { visitor.apply(*this); }

    // provide Array2D<>::accept() implementation
    template<typename T>
    void Array2D<T>::accept(Visitor& visitor) { visitor.apply(*this); }

    // provide Array3D<>::accept() implementation
    template<typename T>
    void Array3D<T>::accept(Visitor& visitor) { visitor.apply(*this); }

} // namespace vsg
