#pragma once

#include <vsg/core/Value.h>
#include <vsg/core/Array.h>

namespace vsg
{

    // forward declare nodes
    class Node;
    class Group;
    class QuadGroup;
    class LOD;
    class Dispatch;
    class RenderPass;

    class Visitor
    {
    public:

        Visitor();

        virtual void apply(Object&);

        // Values
        virtual void apply(StringValue&);
        virtual void apply(IntValue&);
        virtual void apply(UIntValue&);
        virtual void apply(FloatValue&);
        virtual void apply(DoubleValue&);

        // Arrays
        virtual void apply(ushortArray&);
        virtual void apply(uintArray&);
        virtual void apply(floatArray&);
        virtual void apply(doubleArray&);
        virtual void apply(vec2Array&);
        virtual void apply(vec3Array&);
        virtual void apply(vec4Array&);
        virtual void apply(mat4Array&);
        virtual void apply(dvec2Array&);
        virtual void apply(dvec3Array&);
        virtual void apply(dvec4Array&);
        virtual void apply(dmat4Array&);

        // Nodes
        virtual void apply(Node&);
        virtual void apply(Group&);
        virtual void apply(QuadGroup&);
        virtual void apply(LOD&);

        // Vulkan nodes
        virtual void apply(Dispatch&);
        virtual void apply(RenderPass&);
    };


    // provide Value<>::accept() implementation
    template<typename T>
    void Value<T>::accept(Visitor& visitor) { visitor.apply(*this); }

    // provide Array<>::accept() implementation
    template<typename T>
    void Array<T>::accept(Visitor& visitor) { visitor.apply(*this); }

}
