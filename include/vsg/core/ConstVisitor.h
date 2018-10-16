#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Value.h>
#include <vsg/core/Array.h>

namespace vsg
{

    // forward declare nodes
    class Node;
    class Group;
    class QuadGroup;
    class LOD;
    class StateGroup;

    class Command;
    class CommandBuffer;
    class RenderPass;

    class VSG_DECLSPEC ConstVisitor : public Object
    {
    public:

        ConstVisitor();

        virtual void apply(const Object&);

        // Values
        virtual void apply(const stringValue&);
        virtual void apply(const boolValue&);
        virtual void apply(const intValue&);
        virtual void apply(const uintValue&);
        virtual void apply(const floatValue&);
        virtual void apply(const doubleValue&);

        // Arrays
        virtual void apply(const ubyteArray&);
        virtual void apply(const ushortArray&);
        virtual void apply(const uintArray&);
        virtual void apply(const floatArray&);
        virtual void apply(const doubleArray&);
        virtual void apply(const vec2Array&);
        virtual void apply(const vec3Array&);
        virtual void apply(const vec4Array&);
        virtual void apply(const mat4Array&);
        virtual void apply(const dvec2Array&);
        virtual void apply(const dvec3Array&);
        virtual void apply(const dvec4Array&);
        virtual void apply(const dmat4Array&);

        // Nodes
        virtual void apply(const Node&);
        virtual void apply(const Group&);
        virtual void apply(const QuadGroup&);
        virtual void apply(const LOD&);
        virtual void apply(const StateGroup&);

        // Vulkan nodes
        virtual void apply(const Command&);
        virtual void apply(const CommandBuffer&);
        virtual void apply(const RenderPass&);
    };


    // provide Value<>::accept() implementation
    template<typename T>
    void Value<T>::accept(ConstVisitor& visitor) const { visitor.apply(*this); }

    // provide Array<>::accept() implementation
    template<typename T>
    void Array<T>::accept(ConstVisitor& visitor) const { visitor.apply(*this); }

}
