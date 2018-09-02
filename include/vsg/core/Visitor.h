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
    class StateGroup;

    class Command;
    class CommandBuffer;
    class RenderPass;

    class VSG_EXPORT Visitor : public Object
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
        virtual void apply(StateGroup&);

        // Vulkan nodes
        virtual void apply(Command&);
        virtual void apply(CommandBuffer&);
        virtual void apply(RenderPass&);
    };


    // provide Value<>::accept() implementation
    template<typename T>
    void Value<T>::accept(Visitor& visitor) { visitor.apply(*this); }

    // provide Array<>::accept() implementation
    template<typename T>
    void Array<T>::accept(Visitor& visitor) { visitor.apply(*this); }

}
