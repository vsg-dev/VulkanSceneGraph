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
    class JointSampler;
    class MorphSampler;
    class TransformSampler;
    class CameraSampler;
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

    class VSG_DECLSPEC ConstVisitor : public Object
    {
    public:
        ConstVisitor();

        ConstVisitor(const ConstVisitor& rhs, const CopyOp& copyop = {}) :
            Object(rhs, copyop),
            traversalMask(rhs.traversalMask),
            overrideMask(rhs.overrideMask) {}

        Mask traversalMask = MASK_ALL;
        Mask overrideMask = MASK_OFF;

        virtual Instrumentation* getInstrumentation() { return nullptr; }

        virtual void apply(const Object&);
        virtual void apply(const Objects&);
        virtual void apply(const External&);
        virtual void apply(const Data&);

        // Values
        virtual void apply(const stringValue&);
        virtual void apply(const wstringValue&);
        virtual void apply(const boolValue&);
        virtual void apply(const intValue&);
        virtual void apply(const uintValue&);
        virtual void apply(const floatValue&);
        virtual void apply(const doubleValue&);
        virtual void apply(const vec2Value&);
        virtual void apply(const vec3Value&);
        virtual void apply(const vec4Value&);
        virtual void apply(const dvec2Value&);
        virtual void apply(const dvec3Value&);
        virtual void apply(const dvec4Value&);
        virtual void apply(const bvec2Value&);
        virtual void apply(const bvec3Value&);
        virtual void apply(const bvec4Value&);
        virtual void apply(const ubvec2Value&);
        virtual void apply(const ubvec3Value&);
        virtual void apply(const ubvec4Value&);
        virtual void apply(const svec2Value&);
        virtual void apply(const svec3Value&);
        virtual void apply(const svec4Value&);
        virtual void apply(const usvec2Value&);
        virtual void apply(const usvec3Value&);
        virtual void apply(const usvec4Value&);
        virtual void apply(const ivec2Value&);
        virtual void apply(const ivec3Value&);
        virtual void apply(const ivec4Value&);
        virtual void apply(const uivec2Value&);
        virtual void apply(const uivec3Value&);
        virtual void apply(const uivec4Value&);
        virtual void apply(const mat4Value&);
        virtual void apply(const dmat4Value&);

        // Arrays
        virtual void apply(const byteArray&);
        virtual void apply(const ubyteArray&);
        virtual void apply(const shortArray&);
        virtual void apply(const ushortArray&);
        virtual void apply(const intArray&);
        virtual void apply(const uintArray&);
        virtual void apply(const floatArray&);
        virtual void apply(const doubleArray&);
        virtual void apply(const vec2Array&);
        virtual void apply(const vec3Array&);
        virtual void apply(const vec4Array&);
        virtual void apply(const dvec2Array&);
        virtual void apply(const dvec3Array&);
        virtual void apply(const dvec4Array&);
        virtual void apply(const bvec2Array&);
        virtual void apply(const bvec3Array&);
        virtual void apply(const bvec4Array&);
        virtual void apply(const svec2Array&);
        virtual void apply(const svec3Array&);
        virtual void apply(const svec4Array&);
        virtual void apply(const ivec2Array&);
        virtual void apply(const ivec3Array&);
        virtual void apply(const ivec4Array&);
        virtual void apply(const ubvec2Array&);
        virtual void apply(const ubvec3Array&);
        virtual void apply(const ubvec4Array&);
        virtual void apply(const usvec2Array&);
        virtual void apply(const usvec3Array&);
        virtual void apply(const usvec4Array&);
        virtual void apply(const uivec2Array&);
        virtual void apply(const uivec3Array&);
        virtual void apply(const uivec4Array&);
        virtual void apply(const mat4Array&);
        virtual void apply(const dmat4Array&);
        virtual void apply(const block64Array&);
        virtual void apply(const block128Array&);

        // Array2Ds
        virtual void apply(const byteArray2D&);
        virtual void apply(const ubyteArray2D&);
        virtual void apply(const shortArray2D&);
        virtual void apply(const ushortArray2D&);
        virtual void apply(const intArray2D&);
        virtual void apply(const uintArray2D&);
        virtual void apply(const floatArray2D&);
        virtual void apply(const doubleArray2D&);
        virtual void apply(const vec2Array2D&);
        virtual void apply(const vec3Array2D&);
        virtual void apply(const vec4Array2D&);
        virtual void apply(const dvec2Array2D&);
        virtual void apply(const dvec3Array2D&);
        virtual void apply(const dvec4Array2D&);
        virtual void apply(const bvec2Array2D&);
        virtual void apply(const bvec3Array2D&);
        virtual void apply(const bvec4Array2D&);
        virtual void apply(const svec2Array2D&);
        virtual void apply(const svec3Array2D&);
        virtual void apply(const svec4Array2D&);
        virtual void apply(const ivec2Array2D&);
        virtual void apply(const ivec3Array2D&);
        virtual void apply(const ivec4Array2D&);
        virtual void apply(const ubvec2Array2D&);
        virtual void apply(const ubvec3Array2D&);
        virtual void apply(const ubvec4Array2D&);
        virtual void apply(const usvec2Array2D&);
        virtual void apply(const usvec3Array2D&);
        virtual void apply(const usvec4Array2D&);
        virtual void apply(const uivec2Array2D&);
        virtual void apply(const uivec3Array2D&);
        virtual void apply(const uivec4Array2D&);
        virtual void apply(const block64Array2D&);
        virtual void apply(const block128Array2D&);

        // Array3Ds
        virtual void apply(const byteArray3D&);
        virtual void apply(const ubyteArray3D&);
        virtual void apply(const shortArray3D&);
        virtual void apply(const ushortArray3D&);
        virtual void apply(const intArray3D&);
        virtual void apply(const uintArray3D&);
        virtual void apply(const floatArray3D&);
        virtual void apply(const doubleArray3D&);
        virtual void apply(const vec2Array3D&);
        virtual void apply(const vec3Array3D&);
        virtual void apply(const vec4Array3D&);
        virtual void apply(const dvec2Array3D&);
        virtual void apply(const dvec3Array3D&);
        virtual void apply(const dvec4Array3D&);
        virtual void apply(const ubvec2Array3D&);
        virtual void apply(const ubvec3Array3D&);
        virtual void apply(const ubvec4Array3D&);
        virtual void apply(const block64Array3D&);
        virtual void apply(const block128Array3D&);

        // Nodes
        virtual void apply(const Node&);
        virtual void apply(const Compilable&);
        virtual void apply(const Commands&);
        virtual void apply(const Group&);
        virtual void apply(const QuadGroup&);
        virtual void apply(const LOD&);
        virtual void apply(const PagedLOD&);
        virtual void apply(const StateGroup&);
        virtual void apply(const CullGroup&);
        virtual void apply(const CullNode&);
        virtual void apply(const Transform&);
        virtual void apply(const MatrixTransform&);
        virtual void apply(const CoordinateFrame&);
        virtual void apply(const Geometry&);
        virtual void apply(const VertexDraw&);
        virtual void apply(const VertexIndexDraw&);
        virtual void apply(const DepthSorted&);
        virtual void apply(const Layer&);
        virtual void apply(const Bin&);
        virtual void apply(const Switch&);
        virtual void apply(const Light&);
        virtual void apply(const AmbientLight&);
        virtual void apply(const DirectionalLight&);
        virtual void apply(const PointLight&);
        virtual void apply(const SpotLight&);
        virtual void apply(const InstrumentationNode&);
        virtual void apply(const RegionOfInterest&);

        // text
        virtual void apply(const Text&);
        virtual void apply(const TextGroup&);
        virtual void apply(const TextTechnique&);
        virtual void apply(const TextLayout&);

        // animation
        virtual void apply(const Animation&);
        virtual void apply(const AnimationGroup&);
        virtual void apply(const AnimationSampler&);
        virtual void apply(const JointSampler&);
        virtual void apply(const MorphSampler&);
        virtual void apply(const TransformSampler&);
        virtual void apply(const CameraSampler&);
        virtual void apply(const Joint&);

        // Vulkan nodes
        virtual void apply(const BufferInfo&);
        virtual void apply(const ImageInfo&);
        virtual void apply(const ImageView&);
        virtual void apply(const Image&);
        virtual void apply(const Command&);
        virtual void apply(const StateCommand&);
        virtual void apply(const StateSwitch&);
        virtual void apply(const CommandBuffer&);
        virtual void apply(const RenderPass&);
        virtual void apply(const BindDescriptorSet&);
        virtual void apply(const BindDescriptorSets&);
        virtual void apply(const BindViewDescriptorSets&);
        virtual void apply(const Descriptor&);
        virtual void apply(const DescriptorBuffer&);
        virtual void apply(const DescriptorImage&);
        virtual void apply(const DescriptorSet&);
        virtual void apply(const BindVertexBuffers&);
        virtual void apply(const BindIndexBuffer&);
        virtual void apply(const BindComputePipeline&);
        virtual void apply(const BindGraphicsPipeline&);
        virtual void apply(const BindRayTracingPipeline&);
        virtual void apply(const GraphicsPipeline&);
        virtual void apply(const ComputePipeline&);
        virtual void apply(const RayTracingPipeline&);
        virtual void apply(const GraphicsPipelineState&);
        virtual void apply(const ShaderStage&);
        virtual void apply(const VertexInputState&);
        virtual void apply(const InputAssemblyState&);
        virtual void apply(const TessellationState&);
        virtual void apply(const ViewportState&);
        virtual void apply(const RasterizationState&);
        virtual void apply(const MultisampleState&);
        virtual void apply(const DepthStencilState&);
        virtual void apply(const ColorBlendState&);
        virtual void apply(const DynamicState&);
        virtual void apply(const ResourceHints&);
        virtual void apply(const Draw&);
        virtual void apply(const DrawIndexed&);
        virtual void apply(const ClearAttachments&);
        virtual void apply(const ClearColorImage&);
        virtual void apply(const ClearDepthStencilImage&);
        virtual void apply(const QueryPool&);
        virtual void apply(const ResetQueryPool&);
        virtual void apply(const BeginQuery&);
        virtual void apply(const EndQuery&);
        virtual void apply(const WriteTimestamp&);
        virtual void apply(const CopyQueryPoolResults&);

        // mesh shading classes
        virtual void apply(const DrawMeshTasks&);
        virtual void apply(const DrawMeshTasksIndirect&);
        virtual void apply(const DrawMeshTasksIndirectCount&);

        // ui events
        virtual void apply(const UIEvent&);
        virtual void apply(const WindowEvent&);
        virtual void apply(const ExposeWindowEvent&);
        virtual void apply(const ConfigureWindowEvent&);
        virtual void apply(const CloseWindowEvent&);
        virtual void apply(const FocusInEvent&);
        virtual void apply(const FocusOutEvent&);
        virtual void apply(const KeyEvent&);
        virtual void apply(const KeyPressEvent&);
        virtual void apply(const KeyReleaseEvent&);
        virtual void apply(const PointerEvent&);
        virtual void apply(const ButtonPressEvent&);
        virtual void apply(const ButtonReleaseEvent&);
        virtual void apply(const MoveEvent&);
        virtual void apply(const TouchEvent&);
        virtual void apply(const TouchDownEvent&);
        virtual void apply(const TouchUpEvent&);
        virtual void apply(const TouchMoveEvent&);
        virtual void apply(const ScrollWheelEvent&);
        virtual void apply(const TerminateEvent&);
        virtual void apply(const FrameEvent&);

        // utils
        virtual void apply(const ShaderCompileSettings&);

        // viewer
        virtual void apply(const Camera&);
        virtual void apply(const CommandGraph&);
        virtual void apply(const SecondaryCommandGraph&);
        virtual void apply(const RenderGraph&);
        virtual void apply(const View&);
        virtual void apply(const Viewer&);
        virtual void apply(const ViewMatrix&);
        virtual void apply(const LookAt&);
        virtual void apply(const LookDirection&);
        virtual void apply(const RelativeViewMatrix&);
        virtual void apply(const TrackingViewMatrix&);
        virtual void apply(const ProjectionMatrix&);
        virtual void apply(const Perspective&);
        virtual void apply(const Orthographic&);
        virtual void apply(const RelativeProjection&);
        virtual void apply(const EllipsoidPerspective&);

        // general classes
        virtual void apply(const FrameStamp&);

        bool is_compatible(const std::type_info& type) const noexcept override { return typeid(ConstVisitor) == type || Object::is_compatible(type); }
    };

    // provide Value<>::accept() implementation
    template<typename T>
    void Value<T>::accept(ConstVisitor& visitor) const { visitor.apply(*this); }

    // provide Array<>::accept() implementation
    template<typename T>
    void Array<T>::accept(ConstVisitor& visitor) const { visitor.apply(*this); }

    // provide Array2D<>::accept() implementation
    template<typename T>
    void Array2D<T>::accept(ConstVisitor& visitor) const { visitor.apply(*this); }

    // provide Array3D<>::accept() implementation
    template<typename T>
    void Array3D<T>::accept(ConstVisitor& visitor) const { visitor.apply(*this); }

} // namespace vsg
