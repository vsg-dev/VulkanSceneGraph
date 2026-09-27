#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018-2026 Robert Osfield, Chris Djali

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Array.h>
#include <vsg/core/Array2D.h>
#include <vsg/core/Array3D.h>
#include <vsg/core/Mask.h>
#include <vsg/core/Value.h>

#include <optional>

namespace vsg
{
    // forward declare core Objects
    class Objects;
    class External;
    class MipmapLayout;

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
    class InstanceNode;
    class InstanceDraw;
    class InstanceDrawIndexed;

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

    class VSG_DECLSPEC ReplacementVisitor : public Object
    {
    public:
        ReplacementVisitor();

        ReplacementVisitor(const ReplacementVisitor& rhs, const CopyOp& copyop = {}) :
            Object(rhs, copyop),
            traversalMask(rhs.traversalMask),
            overrideMask(rhs.overrideMask) {}

        Mask traversalMask = MASK_ALL;
        Mask overrideMask = MASK_OFF;

        virtual Instrumentation* getInstrumentation() { return nullptr; }

        template<class T>
        std::optional<ref_ptr<T>> acceptChecked(T& object)
        {
            std::optional<ref_ptr<Object>> replacement = object.accept(*this);
            if (!replacement.has_value())
                return std::nullopt;
            if (replacement.value() == nullptr)
                return nullptr;
            if (replacement.value()->is_compatible(typeid(T)))
                return ref_ptr(static_cast<T*>(replacement.value().get()));
            return std::nullopt;
        }

        template<class T>
        bool tryReplacePointer(ref_ptr<T>& ptr)
        {
            std::optional<ref_ptr<T>> newPtr = acceptChecked(*ptr);
            if (newPtr.has_value())
            {
                ptr = newPtr.value();
                return true;
            }
            return false;
        }

        virtual std::optional<ref_ptr<Object>> apply(Object&);
        virtual std::optional<ref_ptr<Object>> apply(Objects&);
        virtual std::optional<ref_ptr<Object>> apply(External&);
        virtual std::optional<ref_ptr<Object>> apply(Data&);
        virtual std::optional<ref_ptr<Object>> apply(MipmapLayout&);

        // Values
        virtual std::optional<ref_ptr<Object>> apply(stringValue&);
        virtual std::optional<ref_ptr<Object>> apply(wstringValue&);
        virtual std::optional<ref_ptr<Object>> apply(boolValue&);
        virtual std::optional<ref_ptr<Object>> apply(intValue&);
        virtual std::optional<ref_ptr<Object>> apply(uintValue&);
        virtual std::optional<ref_ptr<Object>> apply(floatValue&);
        virtual std::optional<ref_ptr<Object>> apply(doubleValue&);
        virtual std::optional<ref_ptr<Object>> apply(vec2Value&);
        virtual std::optional<ref_ptr<Object>> apply(vec3Value&);
        virtual std::optional<ref_ptr<Object>> apply(vec4Value&);
        virtual std::optional<ref_ptr<Object>> apply(dvec2Value&);
        virtual std::optional<ref_ptr<Object>> apply(dvec3Value&);
        virtual std::optional<ref_ptr<Object>> apply(dvec4Value&);
        virtual std::optional<ref_ptr<Object>> apply(bvec2Value&);
        virtual std::optional<ref_ptr<Object>> apply(bvec3Value&);
        virtual std::optional<ref_ptr<Object>> apply(bvec4Value&);
        virtual std::optional<ref_ptr<Object>> apply(ubvec2Value&);
        virtual std::optional<ref_ptr<Object>> apply(ubvec3Value&);
        virtual std::optional<ref_ptr<Object>> apply(ubvec4Value&);
        virtual std::optional<ref_ptr<Object>> apply(svec2Value&);
        virtual std::optional<ref_ptr<Object>> apply(svec3Value&);
        virtual std::optional<ref_ptr<Object>> apply(svec4Value&);
        virtual std::optional<ref_ptr<Object>> apply(usvec2Value&);
        virtual std::optional<ref_ptr<Object>> apply(usvec3Value&);
        virtual std::optional<ref_ptr<Object>> apply(usvec4Value&);
        virtual std::optional<ref_ptr<Object>> apply(ivec2Value&);
        virtual std::optional<ref_ptr<Object>> apply(ivec3Value&);
        virtual std::optional<ref_ptr<Object>> apply(ivec4Value&);
        virtual std::optional<ref_ptr<Object>> apply(uivec2Value&);
        virtual std::optional<ref_ptr<Object>> apply(uivec3Value&);
        virtual std::optional<ref_ptr<Object>> apply(uivec4Value&);
        virtual std::optional<ref_ptr<Object>> apply(mat2Value&);
        virtual std::optional<ref_ptr<Object>> apply(dmat2Value&);
        virtual std::optional<ref_ptr<Object>> apply(mat3Value&);
        virtual std::optional<ref_ptr<Object>> apply(dmat3Value&);
        virtual std::optional<ref_ptr<Object>> apply(mat4Value&);
        virtual std::optional<ref_ptr<Object>> apply(dmat4Value&);

        // Arrays
        virtual std::optional<ref_ptr<Object>> apply(stringArray&);
        virtual std::optional<ref_ptr<Object>> apply(byteArray&);
        virtual std::optional<ref_ptr<Object>> apply(ubyteArray&);
        virtual std::optional<ref_ptr<Object>> apply(shortArray&);
        virtual std::optional<ref_ptr<Object>> apply(ushortArray&);
        virtual std::optional<ref_ptr<Object>> apply(intArray&);
        virtual std::optional<ref_ptr<Object>> apply(uintArray&);
        virtual std::optional<ref_ptr<Object>> apply(floatArray&);
        virtual std::optional<ref_ptr<Object>> apply(doubleArray&);
        virtual std::optional<ref_ptr<Object>> apply(vec2Array&);
        virtual std::optional<ref_ptr<Object>> apply(vec3Array&);
        virtual std::optional<ref_ptr<Object>> apply(vec4Array&);
        virtual std::optional<ref_ptr<Object>> apply(dvec2Array&);
        virtual std::optional<ref_ptr<Object>> apply(dvec3Array&);
        virtual std::optional<ref_ptr<Object>> apply(dvec4Array&);
        virtual std::optional<ref_ptr<Object>> apply(bvec2Array&);
        virtual std::optional<ref_ptr<Object>> apply(bvec3Array&);
        virtual std::optional<ref_ptr<Object>> apply(bvec4Array&);
        virtual std::optional<ref_ptr<Object>> apply(svec2Array&);
        virtual std::optional<ref_ptr<Object>> apply(svec3Array&);
        virtual std::optional<ref_ptr<Object>> apply(svec4Array&);
        virtual std::optional<ref_ptr<Object>> apply(ivec2Array&);
        virtual std::optional<ref_ptr<Object>> apply(ivec3Array&);
        virtual std::optional<ref_ptr<Object>> apply(ivec4Array&);
        virtual std::optional<ref_ptr<Object>> apply(ubvec2Array&);
        virtual std::optional<ref_ptr<Object>> apply(ubvec3Array&);
        virtual std::optional<ref_ptr<Object>> apply(ubvec4Array&);
        virtual std::optional<ref_ptr<Object>> apply(usvec2Array&);
        virtual std::optional<ref_ptr<Object>> apply(usvec3Array&);
        virtual std::optional<ref_ptr<Object>> apply(usvec4Array&);
        virtual std::optional<ref_ptr<Object>> apply(uivec2Array&);
        virtual std::optional<ref_ptr<Object>> apply(uivec3Array&);
        virtual std::optional<ref_ptr<Object>> apply(uivec4Array&);
        virtual std::optional<ref_ptr<Object>> apply(mat4Array&);
        virtual std::optional<ref_ptr<Object>> apply(dmat4Array&);
        virtual std::optional<ref_ptr<Object>> apply(block64Array&);
        virtual std::optional<ref_ptr<Object>> apply(block128Array&);

        // Array2Ds
        virtual std::optional<ref_ptr<Object>> apply(byteArray2D&);
        virtual std::optional<ref_ptr<Object>> apply(ubyteArray2D&);
        virtual std::optional<ref_ptr<Object>> apply(shortArray2D&);
        virtual std::optional<ref_ptr<Object>> apply(ushortArray2D&);
        virtual std::optional<ref_ptr<Object>> apply(intArray2D&);
        virtual std::optional<ref_ptr<Object>> apply(uintArray2D&);
        virtual std::optional<ref_ptr<Object>> apply(floatArray2D&);
        virtual std::optional<ref_ptr<Object>> apply(doubleArray2D&);
        virtual std::optional<ref_ptr<Object>> apply(vec2Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(vec3Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(vec4Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(dvec2Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(dvec3Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(dvec4Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(bvec2Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(bvec3Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(bvec4Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(svec2Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(svec3Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(svec4Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(ivec2Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(ivec3Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(ivec4Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(ubvec2Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(ubvec3Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(ubvec4Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(usvec2Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(usvec3Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(usvec4Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(uivec2Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(uivec3Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(uivec4Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(block64Array2D&);
        virtual std::optional<ref_ptr<Object>> apply(block128Array2D&);

        // Array3Ds
        virtual std::optional<ref_ptr<Object>> apply(byteArray3D&);
        virtual std::optional<ref_ptr<Object>> apply(ubyteArray3D&);
        virtual std::optional<ref_ptr<Object>> apply(shortArray3D&);
        virtual std::optional<ref_ptr<Object>> apply(ushortArray3D&);
        virtual std::optional<ref_ptr<Object>> apply(intArray3D&);
        virtual std::optional<ref_ptr<Object>> apply(uintArray3D&);
        virtual std::optional<ref_ptr<Object>> apply(floatArray3D&);
        virtual std::optional<ref_ptr<Object>> apply(doubleArray3D&);
        virtual std::optional<ref_ptr<Object>> apply(vec2Array3D&);
        virtual std::optional<ref_ptr<Object>> apply(vec3Array3D&);
        virtual std::optional<ref_ptr<Object>> apply(vec4Array3D&);
        virtual std::optional<ref_ptr<Object>> apply(dvec2Array3D&);
        virtual std::optional<ref_ptr<Object>> apply(dvec3Array3D&);
        virtual std::optional<ref_ptr<Object>> apply(dvec4Array3D&);
        virtual std::optional<ref_ptr<Object>> apply(ubvec2Array3D&);
        virtual std::optional<ref_ptr<Object>> apply(ubvec3Array3D&);
        virtual std::optional<ref_ptr<Object>> apply(ubvec4Array3D&);
        virtual std::optional<ref_ptr<Object>> apply(block64Array3D&);
        virtual std::optional<ref_ptr<Object>> apply(block128Array3D&);

        // Nodes
        virtual std::optional<ref_ptr<Object>> apply(Node&);
        virtual std::optional<ref_ptr<Object>> apply(Compilable&);
        virtual std::optional<ref_ptr<Object>> apply(Commands&);
        virtual std::optional<ref_ptr<Object>> apply(Group&);
        virtual std::optional<ref_ptr<Object>> apply(QuadGroup&);
        virtual std::optional<ref_ptr<Object>> apply(LOD&);
        virtual std::optional<ref_ptr<Object>> apply(PagedLOD&);
        virtual std::optional<ref_ptr<Object>> apply(StateGroup&);
        virtual std::optional<ref_ptr<Object>> apply(CullGroup&);
        virtual std::optional<ref_ptr<Object>> apply(CullNode&);
        virtual std::optional<ref_ptr<Object>> apply(Transform&);
        virtual std::optional<ref_ptr<Object>> apply(MatrixTransform&);
        virtual std::optional<ref_ptr<Object>> apply(CoordinateFrame&);
        virtual std::optional<ref_ptr<Object>> apply(Geometry&);
        virtual std::optional<ref_ptr<Object>> apply(VertexDraw&);
        virtual std::optional<ref_ptr<Object>> apply(VertexIndexDraw&);
        virtual std::optional<ref_ptr<Object>> apply(DepthSorted&);
        virtual std::optional<ref_ptr<Object>> apply(Layer&);
        virtual std::optional<ref_ptr<Object>> apply(Bin&);
        virtual std::optional<ref_ptr<Object>> apply(Switch&);
        virtual std::optional<ref_ptr<Object>> apply(Light&);
        virtual std::optional<ref_ptr<Object>> apply(AmbientLight&);
        virtual std::optional<ref_ptr<Object>> apply(DirectionalLight&);
        virtual std::optional<ref_ptr<Object>> apply(PointLight&);
        virtual std::optional<ref_ptr<Object>> apply(SpotLight&);
        virtual std::optional<ref_ptr<Object>> apply(InstrumentationNode&);
        virtual std::optional<ref_ptr<Object>> apply(RegionOfInterest&);
        virtual std::optional<ref_ptr<Object>> apply(InstanceNode&);
        virtual std::optional<ref_ptr<Object>> apply(InstanceDraw&);
        virtual std::optional<ref_ptr<Object>> apply(InstanceDrawIndexed&);

        // text
        virtual std::optional<ref_ptr<Object>> apply(Text&);
        virtual std::optional<ref_ptr<Object>> apply(TextGroup&);
        virtual std::optional<ref_ptr<Object>> apply(TextTechnique&);
        virtual std::optional<ref_ptr<Object>> apply(TextLayout&);

        // animation
        virtual std::optional<ref_ptr<Object>> apply(Animation&);
        virtual std::optional<ref_ptr<Object>> apply(AnimationGroup&);
        virtual std::optional<ref_ptr<Object>> apply(AnimationSampler&);
        virtual std::optional<ref_ptr<Object>> apply(JointSampler&);
        virtual std::optional<ref_ptr<Object>> apply(MorphSampler&);
        virtual std::optional<ref_ptr<Object>> apply(TransformSampler&);
        virtual std::optional<ref_ptr<Object>> apply(CameraSampler&);
        virtual std::optional<ref_ptr<Object>> apply(Joint&);

        // Vulkan nodes
        virtual std::optional<ref_ptr<Object>> apply(BufferInfo&);
        virtual std::optional<ref_ptr<Object>> apply(ImageInfo&);
        virtual std::optional<ref_ptr<Object>> apply(ImageView&);
        virtual std::optional<ref_ptr<Object>> apply(Image&);
        virtual std::optional<ref_ptr<Object>> apply(Command&);
        virtual std::optional<ref_ptr<Object>> apply(StateCommand&);
        virtual std::optional<ref_ptr<Object>> apply(StateSwitch&);
        virtual std::optional<ref_ptr<Object>> apply(CommandBuffer&);
        virtual std::optional<ref_ptr<Object>> apply(RenderPass&);
        virtual std::optional<ref_ptr<Object>> apply(BindDescriptorSet&);
        virtual std::optional<ref_ptr<Object>> apply(BindDescriptorSets&);
        virtual std::optional<ref_ptr<Object>> apply(BindViewDescriptorSets&);
        virtual std::optional<ref_ptr<Object>> apply(Descriptor&);
        virtual std::optional<ref_ptr<Object>> apply(DescriptorBuffer&);
        virtual std::optional<ref_ptr<Object>> apply(DescriptorImage&);
        virtual std::optional<ref_ptr<Object>> apply(DescriptorSet&);
        virtual std::optional<ref_ptr<Object>> apply(BindVertexBuffers&);
        virtual std::optional<ref_ptr<Object>> apply(BindIndexBuffer&);
        virtual std::optional<ref_ptr<Object>> apply(BindComputePipeline&);
        virtual std::optional<ref_ptr<Object>> apply(BindGraphicsPipeline&);
        virtual std::optional<ref_ptr<Object>> apply(BindRayTracingPipeline&);
        virtual std::optional<ref_ptr<Object>> apply(GraphicsPipeline&);
        virtual std::optional<ref_ptr<Object>> apply(ComputePipeline&);
        virtual std::optional<ref_ptr<Object>> apply(RayTracingPipeline&);
        virtual std::optional<ref_ptr<Object>> apply(GraphicsPipelineState&);
        virtual std::optional<ref_ptr<Object>> apply(ShaderStage&);
        virtual std::optional<ref_ptr<Object>> apply(VertexInputState&);
        virtual std::optional<ref_ptr<Object>> apply(InputAssemblyState&);
        virtual std::optional<ref_ptr<Object>> apply(TessellationState&);
        virtual std::optional<ref_ptr<Object>> apply(ViewportState&);
        virtual std::optional<ref_ptr<Object>> apply(RasterizationState&);
        virtual std::optional<ref_ptr<Object>> apply(MultisampleState&);
        virtual std::optional<ref_ptr<Object>> apply(DepthStencilState&);
        virtual std::optional<ref_ptr<Object>> apply(ColorBlendState&);
        virtual std::optional<ref_ptr<Object>> apply(DynamicState&);
        virtual std::optional<ref_ptr<Object>> apply(ResourceHints&);
        virtual std::optional<ref_ptr<Object>> apply(Draw&);
        virtual std::optional<ref_ptr<Object>> apply(DrawIndexed&);
        virtual std::optional<ref_ptr<Object>> apply(ClearAttachments&);
        virtual std::optional<ref_ptr<Object>> apply(ClearColorImage&);
        virtual std::optional<ref_ptr<Object>> apply(ClearDepthStencilImage&);
        virtual std::optional<ref_ptr<Object>> apply(QueryPool&);
        virtual std::optional<ref_ptr<Object>> apply(ResetQueryPool&);
        virtual std::optional<ref_ptr<Object>> apply(BeginQuery&);
        virtual std::optional<ref_ptr<Object>> apply(EndQuery&);
        virtual std::optional<ref_ptr<Object>> apply(WriteTimestamp&);
        virtual std::optional<ref_ptr<Object>> apply(CopyQueryPoolResults&);

        // mesh shading classes
        virtual std::optional<ref_ptr<Object>> apply(DrawMeshTasks&);
        virtual std::optional<ref_ptr<Object>> apply(DrawMeshTasksIndirect&);
        virtual std::optional<ref_ptr<Object>> apply(DrawMeshTasksIndirectCount&);

        // ui events
        virtual std::optional<ref_ptr<Object>> apply(UIEvent&);
        virtual std::optional<ref_ptr<Object>> apply(WindowEvent&);
        virtual std::optional<ref_ptr<Object>> apply(ExposeWindowEvent&);
        virtual std::optional<ref_ptr<Object>> apply(ConfigureWindowEvent&);
        virtual std::optional<ref_ptr<Object>> apply(CloseWindowEvent&);
        virtual std::optional<ref_ptr<Object>> apply(FocusInEvent&);
        virtual std::optional<ref_ptr<Object>> apply(FocusOutEvent&);
        virtual std::optional<ref_ptr<Object>> apply(KeyEvent&);
        virtual std::optional<ref_ptr<Object>> apply(KeyPressEvent&);
        virtual std::optional<ref_ptr<Object>> apply(KeyReleaseEvent&);
        virtual std::optional<ref_ptr<Object>> apply(PointerEvent&);
        virtual std::optional<ref_ptr<Object>> apply(ButtonPressEvent&);
        virtual std::optional<ref_ptr<Object>> apply(ButtonReleaseEvent&);
        virtual std::optional<ref_ptr<Object>> apply(MoveEvent&);
        virtual std::optional<ref_ptr<Object>> apply(TouchEvent&);
        virtual std::optional<ref_ptr<Object>> apply(TouchDownEvent&);
        virtual std::optional<ref_ptr<Object>> apply(TouchUpEvent&);
        virtual std::optional<ref_ptr<Object>> apply(TouchMoveEvent&);
        virtual std::optional<ref_ptr<Object>> apply(ScrollWheelEvent&);
        virtual std::optional<ref_ptr<Object>> apply(TerminateEvent&);
        virtual std::optional<ref_ptr<Object>> apply(FrameEvent&);

        // utils
        virtual std::optional<ref_ptr<Object>> apply(ShaderCompileSettings&);

        // app
        virtual std::optional<ref_ptr<Object>> apply(Camera&);
        virtual std::optional<ref_ptr<Object>> apply(CommandGraph&);
        virtual std::optional<ref_ptr<Object>> apply(SecondaryCommandGraph&);
        virtual std::optional<ref_ptr<Object>> apply(RenderGraph&);
        virtual std::optional<ref_ptr<Object>> apply(View&);
        virtual std::optional<ref_ptr<Object>> apply(Viewer&);
        virtual std::optional<ref_ptr<Object>> apply(ViewMatrix&);
        virtual std::optional<ref_ptr<Object>> apply(LookAt&);
        virtual std::optional<ref_ptr<Object>> apply(LookDirection&);
        virtual std::optional<ref_ptr<Object>> apply(RelativeViewMatrix&);
        virtual std::optional<ref_ptr<Object>> apply(TrackingViewMatrix&);
        virtual std::optional<ref_ptr<Object>> apply(ProjectionMatrix&);
        virtual std::optional<ref_ptr<Object>> apply(Perspective&);
        virtual std::optional<ref_ptr<Object>> apply(Orthographic&);
        virtual std::optional<ref_ptr<Object>> apply(RelativeProjection&);
        virtual std::optional<ref_ptr<Object>> apply(EllipsoidPerspective&);

        // general classes
        virtual std::optional<ref_ptr<Object>> apply(FrameStamp&);

        bool is_compatible(const std::type_info& type) const noexcept override { return typeid(ReplacementVisitor) == type || Object::is_compatible(type); }
    };

    // provide Value<>::accept() implementation
    template<typename T>
    std::optional<ref_ptr<Object>> Value<T>::accept(ReplacementVisitor& visitor) { return visitor.apply(*this); }

    // provide Array<>::accept() implementation
    template<typename T>
    std::optional<ref_ptr<Object>> Array<T>::accept(ReplacementVisitor& visitor) { return visitor.apply(*this); }

    // provide Array2D<>::accept() implementation
    template<typename T>
    std::optional<ref_ptr<Object>> Array2D<T>::accept(ReplacementVisitor& visitor) { return visitor.apply(*this); }

    // provide Array3D<>::accept() implementation
    template<typename T>
    std::optional<ref_ptr<Object>> Array3D<T>::accept(ReplacementVisitor& visitor) { return visitor.apply(*this); }

} // namespace vsg
