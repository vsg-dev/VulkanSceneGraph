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
#include <vsg/core/Value.h>

namespace vsg
{

    // forward declare nodes classes
    class Node;
    class Commands;
    class Group;
    class QuadGroup;
    class LOD;
    class StateGroup;
    class CullGroup;
    class MatrixTransform;
    class Geometry;

    // forward declare vulkan classes
    class Command;
    class CommandBuffer;
    class RenderPass;
    class BindComputePipeline;
    class BindGraphicsPipeline;
    class GraphicsPipeline;
    class ComputePipeline;
    class Draw;
    class DrawIndexed;
    class ShaderStages;
    class GraphicsPipelineState;
    class VertexInputState;
    class InputAssemblyState;
    class ViewportState;
    class RasterizationState;
    class MultisampleState;
    class DepthStencilState;
    class ColorBlendState;

    // forward declare ui events classes
    class UIEvent;
    class WindowEvent;
    class ExposeWindowEvent;
    class ConfigureWindowEvent;
    class CloseWindowEvent;
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
    class TerminateEvent;
    class FrameEvent;

    // forward declare general classes
    class FrameStamp;

    class VSG_DECLSPEC Visitor : public Object
    {
    public:
        Visitor();

        virtual void apply(Object&);

        // Values
        virtual void apply(stringValue&);
        virtual void apply(boolValue&);
        virtual void apply(intValue&);
        virtual void apply(uintValue&);
        virtual void apply(floatValue&);
        virtual void apply(doubleValue&);

        // Arrays
        virtual void apply(ubyteArray&);
        virtual void apply(ushortArray&);
        virtual void apply(uintArray&);
        virtual void apply(floatArray&);
        virtual void apply(doubleArray&);
        virtual void apply(vec2Array&);
        virtual void apply(vec3Array&);
        virtual void apply(vec4Array&);
        virtual void apply(dvec2Array&);
        virtual void apply(dvec3Array&);
        virtual void apply(dvec4Array&);
        virtual void apply(ubvec2Array&);
        virtual void apply(ubvec3Array&);
        virtual void apply(ubvec4Array&);
        virtual void apply(mat4Array&);
        virtual void apply(dmat4Array&);

        // Array2Ds
        virtual void apply(ubyteArray2D&);
        virtual void apply(ushortArray2D&);
        virtual void apply(uintArray2D&);
        virtual void apply(floatArray2D&);
        virtual void apply(doubleArray2D&);
        virtual void apply(vec2Array2D&);
        virtual void apply(vec3Array2D&);
        virtual void apply(vec4Array2D&);
        virtual void apply(dvec2Array2D&);
        virtual void apply(dvec3Array2D&);
        virtual void apply(dvec4Array2D&);
        virtual void apply(ubvec2Array2D&);
        virtual void apply(ubvec3Array2D&);
        virtual void apply(ubvec4Array2D&);

        // Array3Ds
        virtual void apply(ubyteArray3D&);
        virtual void apply(ushortArray3D&);
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

        // Nodes
        virtual void apply(Node&);
        virtual void apply(Commands&);
        virtual void apply(Group&);
        virtual void apply(QuadGroup&);
        virtual void apply(LOD&);
        virtual void apply(StateGroup&);
        virtual void apply(CullGroup&);
        virtual void apply(MatrixTransform&);
        virtual void apply(Geometry&);

        // Vulkan nodes
        virtual void apply(Command&);
        virtual void apply(CommandBuffer&);
        virtual void apply(RenderPass&);
        virtual void apply(BindComputePipeline&);
        virtual void apply(BindGraphicsPipeline&);
        virtual void apply(GraphicsPipeline&);
        virtual void apply(ComputePipeline&);
        virtual void apply(GraphicsPipelineState&);
        virtual void apply(ShaderStages&);
        virtual void apply(VertexInputState&);
        virtual void apply(InputAssemblyState&);
        virtual void apply(ViewportState&);
        virtual void apply(RasterizationState&);
        virtual void apply(MultisampleState&);
        virtual void apply(DepthStencilState&);
        virtual void apply(ColorBlendState&);

        // ui events
        virtual void apply(UIEvent&);
        virtual void apply(WindowEvent&);
        virtual void apply(ExposeWindowEvent&);
        virtual void apply(ConfigureWindowEvent&);
        virtual void apply(CloseWindowEvent&);
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
        virtual void apply(TerminateEvent&);
        virtual void apply(FrameEvent&);

        // general classes
        virtual void apply(FrameStamp&);
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
